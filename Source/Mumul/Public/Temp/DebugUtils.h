#pragma once

// MACRO : RPC
// LOG_RPC("")
#define LOG_RPC(Format, ...) \
{ \
const TCHAR* NM = \
(GetNetMode() == NM_Client) ? TEXT("CLIENT") : \
(GetNetMode() == NM_ListenServer) ? TEXT("LISTEN_SERVER") : \
(GetNetMode() == NM_DedicatedServer) ? TEXT("DEDICATED_SERVER") : TEXT("UNKNOWN"); \
\
APlayerController* PC_ = Cast<APlayerController>(this); \
APlayerState* PS_ = PC_ ? PC_->PlayerState : nullptr; \
\
float T = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.f; \
\
UE_LOG(LogTemp, Warning, TEXT("[%.2f][%s][%s] PC=%s PSID=%d : " Format), \
T, NM, TEXT(__FUNCTION__), \
PC_ ? *PC_->GetName() : TEXT("NULL"), \
PS_ ? PS_->GetPlayerId() : -1, \
##__VA_ARGS__); \
}
// MACRO : ARRAY
// LOG_ARRAY(Array)
#define LOG_ARRAY(Arr) \
{ \
FString __LogMsg = TEXT("[ "); \
for (int32 i = 0; i < Arr.Num(); i++) \
{ \
__LogMsg += FString::Printf(TEXT("%s, "), *LexToString(Arr[i])); \
} \
__LogMsg += TEXT(" ]"); \
UE_LOG(LogTemp, Warning, TEXT("%s"), *__LogMsg); \
}