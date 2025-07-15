#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/NetLib.h>
#include <Library/HttpLib.h>
#include <Protocol/Http.h>
#include <Protocol/ServiceBinding.h>

EFI_STATUS
EFIAPI
UefiMain(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
  EFI_STATUS                Status;
  EFI_HTTP_PROTOCOL         *Http;
  EFI_HTTP_CONFIG_DATA      HttpConfigData;
  EFI_HTTP_TOKEN            RequestToken;
  EFI_HTTP_REQUEST_DATA     RequestData;
  EFI_HTTP_MESSAGE          RequestMessage;
  EFI_HTTP_RESPONSE_DATA    *ResponseData;
  EFI_HTTP_HEADER           RequestHeaders[4];
  CHAR8                     *JsonBody;
  UINTN                     JsonLength;
  CHAR8                     ResponseBuffer[1024];
  UINTN                     ResponseLength;
  
  // 设置控制台输出
  SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
  Print(L"Starting HTTP POST example in UEFI environment...\n\n");
  
  // ===== 1. 初始化HTTP协议 =====
  Status = NetLibCreateServiceChild(
             ImageHandle,
             gImageHandle,
             &gEfiHttpServiceBindingProtocolGuid,
             &ImageHandle);
  if (EFI_ERROR(Status)) {
    Print(L"Failed to create HTTP service child: %r\n", Status);
    return Status;
  }
  
  // 获取HTTP协议实例
  Status = gBS->OpenProtocol(
             ImageHandle,
             &gEfiHttpProtocolGuid,
             (VOID **)&Http,
             ImageHandle,
             NULL,
             EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (EFI_ERROR(Status)) {
    Print(L"Failed to open HTTP protocol: %r\n", Status);
    return Status;
  }
  
  // 配置HTTP协议
  ZeroMem(&HttpConfigData, sizeof(HttpConfigData));
  HttpConfigData.HttpVersion = HttpVersion11;
  Status = Http->Configure(Http, &HttpConfigData);
  if (EFI_ERROR(Status)) {
    Print(L"Failed to configure HTTP protocol: %r\n", Status);
    goto ErrorExit;
  }
  
  // ===== 2. 准备HTTP请求 =====
  // 构造JSON请求体
  JsonBody = "{\"algo\": \"sm2\",\"kms\": \"\",\"flow\": \"classic\"}";
  JsonLength = AsciiStrLen(JsonBody);
  
  // 设置请求头
  RequestHeaders[0].FieldName = (CHAR8 *)"Host";
  RequestHeaders[0].FieldValue = (CHAR8 *)"10.20.173.8";
  
  RequestHeaders[1].FieldName = (CHAR8 *)"User-Agent";
  RequestHeaders[1].FieldValue = (CHAR8 *)"UEFI-HTTP-Client/1.0";
  
  RequestHeaders[2].FieldName = (CHAR8 *)"Content-Type";
  RequestHeaders[2].FieldValue = (CHAR8 *)"application/json";
  
  RequestHeaders[3].FieldName = (CHAR8 *)"Content-Length";
  // 注意：需要将数字转换为字符串
  CHAR8 ContentLengthStr[20];
  AsciiSPrint(ContentLengthStr, sizeof(ContentLengthStr), "%d", JsonLength);
  RequestHeaders[3].FieldValue = ContentLengthStr;
  
  // 设置请求数据
  ZeroMem(&RequestData, sizeof(RequestData));
  RequestData.Method = HttpMethodPost;
  RequestData.Url = (CHAR8 *)"http://10.20.173.8:80/v1/keypair";
  
  // 设置请求消息
  ZeroMem(&RequestMessage, sizeof(RequestMessage));
  RequestMessage.Data.Request = &RequestData;
  RequestMessage.HeaderCount = 4;
  RequestMessage.Headers = RequestHeaders;
  RequestMessage.BodyLength = JsonLength;
  RequestMessage.Body = (VOID *)JsonBody;
  
  // ===== 3. 设置HTTP令牌 =====
  ZeroMem(&RequestToken, sizeof(RequestToken));
  RequestToken.Event = NULL; // 同步操作
  RequestToken.Message = &RequestMessage;
  
  // 设置响应缓冲区
  ResponseLength = sizeof(ResponseBuffer);
  ZeroMem(ResponseBuffer, ResponseLength);
  
  // ===== 4. 发送HTTP请求 =====
  Print(L"Sending HTTP POST request to 10.20.173.8/v1/key...\n");
  Status = Http->Request(Http, &RequestToken );
  if (EFI_ERROR(Status)) {
    Print(L"HTTP request failed: %r\n", Status);
    goto ErrorExit;
  }
  
  // ===== 5. 处理HTTP响应 =====
  if (RequestToken.Status == EFI_SUCCESS) {
    ResponseData = RequestToken.Message->Data.Response;
    
    // 确保响应缓冲区以null结尾
    UINTN CopySize = MIN(ResponseLength - 1, RequestToken.Message->BodyLength);
    CopyMem(ResponseBuffer, RequestToken.Message->Body, CopySize);
    ResponseBuffer[CopySize] = '\0';
    
    // Print(L"\nHTTP Response Status: %d %a\n", 
    //       ResponseData->StatusCode, 
    //       ResponseData->ReasonPhrase);
    
    // 打印响应头
    Print(L"Response Headers:\n");
    for (UINTN i = 0; i < RequestToken.Message->HeaderCount; i++) {
      Print(L"  %a: %a\n", 
            RequestToken.Message->Headers[i].FieldName,
            RequestToken.Message->Headers[i].FieldValue);
    }
    
    // 打印响应体
    Print(L"\nResponse Body:\n%a\n", ResponseBuffer);
  } else {
    Print(L"HTTP request completed with error: %r\n", RequestToken.Status);
  }
  
ErrorExit:
  // ===== 6. 清理资源 =====
  // 关闭HTTP协议
  if (Http != NULL) {
    Http->Configure(Http, NULL); // 取消配置
    gBS->CloseProtocol(
           ImageHandle,
           &gEfiHttpProtocolGuid,
           ImageHandle,
           NULL);
  }
  
  // 销毁服务子项
  NetLibDestroyServiceChild(
    ImageHandle,
    gImageHandle,
    &gEfiHttpServiceBindingProtocolGuid,
    ImageHandle);
  
  Print(L"\nHTTP example completed. Press any key to exit...\n");
  SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
  WaitForSingleEvent(SystemTable->ConIn->WaitForKey, 0);
  
  return EFI_SUCCESS;
}