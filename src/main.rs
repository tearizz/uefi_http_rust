use anyhow::{anyhow, Result};
use reqwest::Response;
use serde::{Deserialize, Serialize};
use std::time::Duration;
use std::fs::File;
use std::io::Write;

// 请求数据结构（匹配您提供的请求格式）
#[derive(Debug, Serialize)]
struct RequestPayload {
    algo: String,
    kms: String,
    flow: String,
}

// 响应数据结构（精确匹配您提供的响应格式）
#[derive(Debug, Deserialize)]
struct ApiResponse {
    base_config: BaseConfig,
    #[serde(rename = "priv")]
    private_key: String,  // 使用重命名处理关键字冲突
    #[serde(rename = "pub")]
    public_key: String,   // 使用重命名处理关键字冲突
    key_id: String,
}

#[derive(Debug, Deserialize)]
struct BaseConfig {
    algo: String,
    kms: String,
    flow: String,
}

#[tokio::main]
async fn main() -> Result<()> {
    // 目标API端点（替换为您的实际API地址）
    let api_url = "http://10.20.173.8:80/v1/keypair";
    
    println!("正在发送POST请求到: {}", api_url);
    
    // 创建请求体（精确匹配您提供的JSON）
    let payload = RequestPayload {
        algo: "sm2".to_string(),
        kms: "".to_string(),
        flow: "classic".to_string(),
    };
    
    // 打印请求详情
    println!("\n请求体内容:");
    let payload_json = serde_json::to_string_pretty(&payload)?;
    println!("{}", payload_json);
    
    // 创建HTTP客户端
    let client = reqwest::Client::builder()
        .timeout(Duration::from_secs(30))
        .user_agent("Rust Crypto API Client/1.0")
        .build()?;
    
    // 发送POST请求
    let response = client
        .post(api_url)
        .header("Content-Type", "application/json")
        .json(&payload)
        .send()
        .await?;
    
    // 处理响应
    println!("\n响应状态: {}", response.status());
    
    // 检查状态码是否成功
    if !response.status().is_success() {
        return handle_error_response(response).await;
    }
    
    // 解析响应
    let api_response: ApiResponse = response.json().await?;
    
    // 打印解析结果
    print_response(&api_response)?;
    
    // 保存密钥到文件
    save_keys_to_files(&api_response)?;
    
    Ok(())
}

/// 打印API响应详情
fn print_response(response: &ApiResponse) -> Result<()> {
    println!("\n API响应解析成功:");
    
    println!("\nbase_config:");
    println!("  算法: {}", response.base_config.algo);
    println!("  KMS: {}", response.base_config.kms);
    println!("  流程: {}", response.base_config.flow);
    
    println!("\n私钥:");
    println!("{}", response.private_key);
    
    println!("\n公钥:");
    println!("{}", response.public_key);
    
    println!("\n密钥ID: {}", response.key_id);
    
    Ok(())
}

/// 将密钥保存到文件
fn save_keys_to_files(response: &ApiResponse) -> Result<()> {
    // 创建私钥文件
    let private_key_filename = format!("{}.pem", response.key_id);
    let mut private_key_file = File::create(&private_key_filename)?;
    private_key_file.write_all(response.private_key.as_bytes())?;
    println!("\n私钥已保存到: {}", private_key_filename);
    
    // 创建公钥文件
    let public_key_filename = format!("{}.pub", response.key_id);
    let mut public_key_file = File::create(&public_key_filename)?;
    public_key_file.write_all(response.public_key.as_bytes())?;
    println!("公钥已保存到: {}", public_key_filename);
    
    // 创建元数据文件
    let metadata_filename = format!("{}_metadata.json", response.key_id);
    let metadata = serde_json::json!({
        "key_id": response.key_id,
        "algo": response.base_config.algo,
        "flow": response.base_config.flow,
        "generated_at": chrono::Utc::now().to_rfc3339(),
    });
    let mut metadata_file = File::create(&metadata_filename)?;
    metadata_file.write_all(serde_json::to_string_pretty(&metadata)?.as_bytes())?;
    println!("元数据已保存到: {}", metadata_filename);
    
    Ok(())
}

/// 处理错误响应
async fn handle_error_response(response: Response) -> Result<()> {
    let status = response.status();
    let error_text = response.text().await?;
    
    println!("\n请求失败! 状态码: {}", status);
    
    // 尝试解析错误响应
    if let Ok(error_json) = serde_json::from_str::<serde_json::Value>(&error_text) {
        println!("错误详情:");
        println!("{}", serde_json::to_string_pretty(&error_json)?);
    } else {
        println!("原始错误响应:\n{}", error_text);
    }
    
    Err(anyhow!("API请求失败: {}", status))
}