
use anyhow::Result;
use clap::Parser;
use urukhai::{UrukClient, UrukConfig, ChecksumType, Compression};

#[derive(Parser, Debug)]
#[command(name = "urukhai-client", about = "Uruk-hai Binary Protocol Client")]
struct Cli {
    #[arg(short, long, default_value = "localhost:8080")]
    addr: String,

    #[arg(short, long, default_value = "default-key")]
    key: String,

    #[arg(short, long)]
    message: String,
}

#[tokio::main]
async fn main() -> Result<()> {
    env_logger::init();
    let cli = Cli::parse();

    let config = UrukConfig {
        checksum_type: ChecksumType::Crc32,
        use_aes: false,
        key: None,
        compression: Compression::None,
    };

    let client = UrukClient::connect(&cli.addr, config).await?;
    log::info!("✅ Connected to Uruk-hai Tower at {}", &cli.addr);

    let heartbeat_client = client.clone();
    tokio::spawn(async move {
        heartbeat_client.run_heartbeat().await;
    });

    log::info!("▶️ Sending message: '{}' with key '{}'", &cli.message, &cli.key);

    client.push(&cli.key, cli.message.as_bytes()).await?;

    log::info!("✅ Message sent successfully.");

    Ok(())
}
