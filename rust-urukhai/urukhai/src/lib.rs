
use aes_gcm::{
    aead::{Aead, KeyInit},
    Aes256Gcm, Key, Nonce,
};
use crc32fast::hash as crc32_hash;
use lz4_flex::{compress_prepend_size, decompress_size_prepended};
use memmap2::MmapMut;
use sha1::{Digest, Sha1};
use std::collections::HashMap;
use std::fs::OpenOptions;
use std::path::Path;
use std::sync::Arc;
use thiserror::Error;
use tokio::io::AsyncWriteExt;
use tokio::net::TcpStream;
use tokio::sync::Mutex;
use tokio::time::{interval, Duration};

#[derive(Error, Debug)]
pub enum UrukError {
    #[error("I/O error: {0}")]
    Io(#[from] std::io::Error),
    #[error("Invalid URUK header")]
    InvalidHeader,
    #[error("AES enabled but no key provided")]
    AesKeyMissing,
    #[error("Unexpected end of file")]
    UnexpectedEof,
    #[error("CRC32 mismatch")]
    Crc32Mismatch,
    #[error("SHA-1 mismatch")]
    Sha1Mismatch,
    #[error("Data too short for AES (need Nonce)")]
    DataTooShortForAes,
    #[error("AES decryption failed")]
    AesDecryptionFailed,
    #[error("Encryption error")]
    EncryptionError,
    #[error("Invalid file path")]
    InvalidPath,
    #[error("Decompression error: {0}")]
    DecompressionError(String),
    #[error("Corrupted block: Expected {expected:08X}, got {actual:08X}")]
    CorruptedBlock { expected: u32, actual: u32 },
    #[error("Failed to recover block at offset {0}")]
    RecoveryFailed(usize),
    #[error("Patch failed: {0}")]
    PatchFailed(String),
    #[error("Base block not found for diff")]
    BaseBlockNotFound,
}

#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum UrukTag {
    Void = 0x00,
    Bone = 0x01,
    Iron = 0x02,
    Blood = 0x04,
    Raw = 0x10,
    Horde = 0x20,
    Warband = 0x30,
    OrcBlood = 0x40,
}

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum ChecksumType {
    None,
    Crc32,
    Sha1,
}

#[derive(Debug, Clone, Copy)]
pub enum Compression {
    None,
    Lz4,
    Zstd,
}
#[derive(Debug, Clone)]
pub struct UrukConfig {
    pub checksum_type: ChecksumType,
    pub use_aes: bool,
    pub key: Option<[u8; 32]>,
    pub compression: Compression,
}

pub struct UrukElement<'a> {
    pub tag: UrukTag,
    pub data: &'a [u8],
}

pub struct UrukOwnedElement {
    pub tag: UrukTag,
    pub payload: Vec<u8>,
}

pub struct UrukParser<'a> {
    buffer: &'a [u8],
    offset: usize,
    config: UrukConfig,
}

impl<'a> UrukParser<'a> {
    pub fn new(buffer: &'a [u8], key: Option<[u8; 32]>) -> Result<Self, UrukError> {
        if buffer.len() < 8 || &buffer[0..4] != b"URUK" {
            return Err(UrukError::InvalidHeader);
        }

        let flags = buffer[6];

        let checksum_type = if (flags & 0x01) == 0 {
            ChecksumType::None
        } else if (flags & 0x02) == 0 {
            ChecksumType::Crc32
        } else {
            ChecksumType::Sha1
        };

        let compression = match (flags & 0x08 != 0, flags & 0x10 != 0) {
            (true, _) => Compression::Lz4,
            (_, true) => Compression::Zstd,
            _ => Compression::None,
        };

        let config = UrukConfig {
            checksum_type,
            use_aes: (flags & 0x04) != 0,
            key,
            compression,
        };

        if config.use_aes && config.key.is_none() {
            return Err(UrukError::AesKeyMissing);
        }

        Ok(Self {
            buffer,
            offset: 8,
            config,
        })
    }

    pub fn next_element(&mut self) -> Option<Result<UrukOwnedElement, UrukError>> {
        if self.offset >= self.buffer.len() {
            return None;
        }

        let tag = self.buffer[self.offset];
        let data_len =
            u32::from_le_bytes(self.buffer[self.offset + 1..self.offset + 5].try_into().ok()?)
                as usize;

        let data_start = self.offset + 5;
        let data_end = data_start + data_len;

        if data_end > self.buffer.len() {
            return Some(Err(UrukError::UnexpectedEof));
        }

        let raw_data = &self.buffer[data_start..data_end];
        let mut current_ptr = data_end;

        match self.config.checksum_type {
            ChecksumType::Crc32 => {
                let stored =
                    u32::from_le_bytes(self.buffer[current_ptr..current_ptr + 4].try_into().ok()?);
                if crc32_hash(raw_data) != stored {
                    return Some(Err(UrukError::Crc32Mismatch));
                }
                current_ptr += 4;
            }
            ChecksumType::Sha1 => {
                let stored = &self.buffer[current_ptr..current_ptr + 20];
                let mut hasher = Sha1::new();
                hasher.update(raw_data);
                if hasher.finalize().as_slice() != stored {
                    return Some(Err(UrukError::Sha1Mismatch));
                }
                current_ptr += 20;
            }
            ChecksumType::None => {}
        }

        let final_payload = if self.config.use_aes {
            let cipher = Aes256Gcm::new(Key::<Aes256Gcm>::from_slice(&self.config.key.unwrap()));
            if raw_data.len() < 12 {
                return Some(Err(UrukError::DataTooShortForAes));
            }

            let nonce = Nonce::from_slice(&raw_data[0..12]);
            let ciphertext = &raw_data[12..];

            match cipher.decrypt(nonce, ciphertext) {
                Ok(decrypted) => decrypted,
                Err(_) => return Some(Err(UrukError::AesDecryptionFailed)),
            }
        } else {
            raw_data.to_vec()
        };

        let decompressed_payload = match self.config.compression {
            Compression::Lz4 => decompress_size_prepended(&final_payload)
                .map_err(|e| UrukError::DecompressionError(e.to_string())),
            Compression::Zstd => zstd::decode_all(&final_payload[..])
                .map_err(|e| UrukError::DecompressionError(e.to_string())),
            Compression::None => Ok(final_payload),
        };

        self.offset = current_ptr;

        Some(decompressed_payload.map(|p| UrukOwnedElement {
            tag: unsafe { std::mem::transmute(tag) },
            payload: p,
        }))
    }
}
pub struct UrukWriter {
    mmap: MmapMut,
    cursor: usize,
    config: UrukConfig,
}

impl UrukWriter {
    pub fn new<P: AsRef<Path>>(
        path: P,
        size: u64,
        config: UrukConfig,
    ) -> Result<Self, UrukError> {
        let file = OpenOptions::new()
            .read(true)
            .write(true)
            .create(true)
            .open(path)?;

        file.set_len(size)?;
        let mut mmap = unsafe { MmapMut::map_mut(&file)? };

        mmap[0..4].copy_from_slice(b"URUK");
        mmap[4..6].copy_from_slice(&[0, 1]); // Breed (Version)

        let mut flags: u8 = 0;
        match config.checksum_type {
            ChecksumType::Crc32 => flags |= 0x01,
            ChecksumType::Sha1 => flags |= 0x01 | 0x02,
            ChecksumType::None => (),
        }
        if config.use_aes {
            flags |= 0x04;
        }
        match config.compression {
            Compression::Lz4 => flags |= 0x08,
            Compression::Zstd => flags |= 0x10,
            Compression::None => (),
        }
        mmap[6] = flags;

        Ok(Self {
            mmap,
            cursor: 8,
            config,
        })
    }

    pub fn write_block(&mut self, tag: UrukTag, data: &[u8]) -> Result<(), UrukError> {
        let compressed_data = match self.config.compression {
            Compression::Lz4 => compress_prepend_size(data),
            Compression::Zstd => zstd::encode_all(data, 3)?,
            Compression::None => data.to_vec(),
        };

        let payload = if self.config.use_aes {
            let cipher = Aes256Gcm::new(Key::<Aes256Gcm>::from_slice(
                &self.config.key.ok_or(UrukError::AesKeyMissing)?,
            ));
            let nonce_bytes = rand::random::<[u8; 12]>();
            let nonce = Nonce::from_slice(&nonce_bytes);
            let ciphertext = cipher
                .encrypt(nonce, compressed_data.as_slice())
                .map_err(|_| UrukError::EncryptionError)?;

            let mut combined = nonce_bytes.to_vec();
            combined.extend(ciphertext);
            combined
        } else {
            compressed_data
        };

        let len = payload.len() as u32;

        self.mmap[self.cursor] = tag as u8;
        self.mmap[self.cursor + 1..self.cursor + 5].copy_from_slice(&len.to_le_bytes());
        self.cursor += 5;

        let end_data = self.cursor + payload.len();
        self.mmap[self.cursor..end_data].copy_from_slice(&payload);

        match self.config.checksum_type {
            ChecksumType::Crc32 => {
                let crc = crc32_hash(&payload);
                self.mmap[end_data..end_data + 4].copy_from_slice(&crc.to_le_bytes());
                self.cursor = end_data + 4;
            }
            ChecksumType::Sha1 => {
                let mut hasher = Sha1::new();
                hasher.update(&payload);
                self.mmap[end_data..end_data + 20].copy_from_slice(&hasher.finalize());
                self.cursor = end_data + 20;
            }
            ChecksumType::None => {
                self.cursor = end_data;
            }
        }

        Ok(())
    }

    pub fn finish(self) -> Result<(), UrukError> {
        self.mmap.flush()?;
        Ok(())
    }
}
#[derive(Clone)]
pub struct UrukClient {
    stream: Arc<Mutex<TcpStream>>,
    config: UrukConfig,
    cache: Arc<Mutex<HashMap<String, Vec<u8>>>>,
}

impl UrukClient {
    pub async fn connect(addr: &str, config: UrukConfig) -> Result<Self, UrukError> {
        let stream = TcpStream::connect(addr).await?;
        Ok(Self {
            stream: Arc::new(Mutex::new(stream)),
            config,
            cache: Arc::new(Mutex::new(HashMap::new())),
        })
    }

    pub async fn push(&self, key: &str, data: &[u8]) -> tokio::io::Result<()> {
        let mut cache = self.cache.lock().await;
        let data_changed = if let Some(old_data) = cache.get(key) {
            old_data != data
        } else {
            true
        };

        if data_changed {
            self.send_raw(data).await?;
            cache.insert(key.to_string(), data.to_vec());
        }

        Ok(())
    }

    pub async fn run_heartbeat(&self) {
        let mut ticker = interval(Duration::from_secs(20));
        loop {
            ticker.tick().await;
            let mut stream = self.stream.lock().await;
            let _ = stream
                .write_all(&[UrukTag::Void as u8, 0, 0, 0, 0])
                .await;
        }
    }

    async fn send_raw(&self, data: &[u8]) -> tokio::io::Result<()> {
        let len = data.len() as u32;
        let mut buf = vec![UrukTag::Raw as u8];
        buf.extend_from_slice(&len.to_le_bytes());
        buf.extend_from_slice(data);

        match self.config.checksum_type {
            ChecksumType::Crc32 => {
                let crc = crc32_hash(data);
                buf.extend_from_slice(&crc.to_le_bytes());
            }
            ChecksumType::Sha1 => {
                let mut hasher = Sha1::new();
                hasher.update(data);
                buf.extend(hasher.finalize());
            }
            ChecksumType::None => {}
        }

        let mut stream = self.stream.lock().await;
        stream.write_all(&buf).await
    }
}
