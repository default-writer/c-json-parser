
use anyhow::Result;
use clap::Parser;
use std::convert::TryInto;
use tokio::io::AsyncReadExt;
use tokio::net::{TcpListener, TcpStream};
use tokio::time::{timeout, Duration};
use urukhai::{UrukError, UrukTag};

const HEARTBEAT_INTERVAL: Duration = Duration::from_secs(30);
const MAX_PAYLOAD_SIZE: usize = 10 * 1024 * 1024; // 10MB limit

#[derive(Parser, Debug)]
#[command(name = "urukhai-server", about = "Uruk-hai Binary Protocol Server")]
struct Cli {
    #[arg(short, long, default_value = "localhost:8080")]
    addr: String,
}

#[tokio::main]
async fn main() -> Result<()> {
    env_logger::init();
    let cli = Cli::parse();

    let listener = TcpListener::bind(&cli.addr).await?;
    log::info!("⚔️ Uruk-hai Tower listening on {}", &cli.addr);

    loop {
        let (socket, addr) = listener.accept().await?;
        log::info!("📥 New recruit from {}", addr);

        tokio::spawn(async move {
            if let Err(e) = handle_uruk_warrior(socket).await {
                log::error!("❌ Warrior from {} has fallen: {}", addr, e);
            } else {
                log::info!("👋 Warrior from {} has left the battle.", addr);
            }
        });
    }
}

async fn handle_uruk_warrior(mut socket: TcpStream) -> Result<(), Box<dyn std::error::Error>> {
    let mut header = [0u8; 5];

    loop {
        let read_result = timeout(HEARTBEAT_INTERVAL, socket.read_exact(&mut header)).await;

        match read_result {
            Ok(Ok(_)) => {
                log::debug!("Received header bytes: {:?}", &header);
                let tag_byte = header[0];
                let tag = unsafe { std::mem::transmute(tag_byte) };
                let len = u32::from_le_bytes(header[1..5].try_into()?) as usize;

                log::debug!("Header received: tag={:?}, len={}", tag, len);

                if tag == UrukTag::Void {
                    log::debug!("❤️ Heartbeat received");
                    continue;
                }

                if len > MAX_PAYLOAD_SIZE {
                    return Err(Box::new(UrukError::UnexpectedEof));
                }

                let mut buffer = vec![0u8; len + 4]; // +4 for CRC
                socket.read_exact(&mut buffer).await?;
                log::debug!("Received payload bytes: {:?}", &buffer);

                let (payload, crc_bytes) = buffer.split_at(len);
                let stored_crc = u32::from_le_bytes(crc_bytes.try_into()?);

                if crc32fast::hash(payload) != stored_crc {
                    log::warn!("⚠️ Corrupted data received. Discarding package.");
                    continue;
                }

                process_data(tag, payload);
            }
            Ok(Err(e)) => {
                if e.kind() == std::io::ErrorKind::UnexpectedEof {
                    // Clean disconnect
                    break;
                }
                // Other socket error
                return Err(e.into());
            }
            Err(_) => {
                // Timeout
                log::warn!("💀 Warrior timed out. Removing from the ranks.");
                break;
            }
        }
    }
    Ok(())
}

fn process_data(tag: UrukTag, payload: &[u8]) {
    match tag {
        UrukTag::Raw => {
            log::info!("📜 Received RAW block ({} bytes)", payload.len());
            // Here you would handle the raw data, e.g., save to a file or database.
        }
        UrukTag::OrcBlood => {
            log::info!("🩸 Received ORC_BLOOD (diff) block ({} bytes)", payload.len());
            // Here you would apply the diff to the base data.
        }
        _ => {
            log::warn!("❓ Received unknown artifact with tag: {:02x}", tag as u8);
        }
    }
}
