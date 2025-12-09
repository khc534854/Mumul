// Fill out your copyright notice in the Description page of Project Settings.


#include "Temp/DBNetwork.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Temp/DBData.h"

void UDBNetwork::SendCreateUserRequest(const FString& UserName, int32 Score)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("user_id"), UserName);
	JsonObject->SetNumberField(TEXT("score"), Score);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(TEXT("http://127.0.0.1:8000/add"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(OutputString);
	Request->OnProcessRequestComplete().BindUObject(this, &UDBNetwork::OnResponseReceived);
	Request->ProcessRequest();
}

void UDBNetwork::GetAllData()
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("http://127.0.0.1:8000/data"));
    Request->SetVerb(TEXT("GET"));
    Request->OnProcessRequestComplete().BindUObject(this, &UDBNetwork::OnResponseReceived);
    Request->ProcessRequest();
}

void UDBNetwork::GetUserData(const FString& UserID)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    FString URL = FString::Printf(TEXT("http://127.0.0.1:8000/data/%s"), *UserID);
    Request->SetURL(URL);
    Request->SetVerb(TEXT("GET"));
    Request->OnProcessRequestComplete().BindUObject(this, &UDBNetwork::OnResponseReceived);
    Request->ProcessRequest();
}

void UDBNetwork::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() != 200)
    {
        UE_LOG(LogTemp, Error, TEXT("Request failed"));
        return;
    }

    FString ResponseStr = Response->GetContentAsString();

    // 배열인지 단일 객체인지 구분
    if (ResponseStr.StartsWith(TEXT("["))) // 배열
    {
        TArray<TSharedPtr<FJsonValue>> JsonArray;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

        if (FJsonSerializer::Deserialize(Reader, JsonArray))
        {
            TArray<FDBData> Users;
            for (TSharedPtr<FJsonValue> Value : JsonArray)
            {
                TSharedPtr<FJsonObject> Obj = Value->AsObject();
                if (Obj.IsValid())
                {
                    FDBData User;
                    User.UserID = Obj->GetStringField(TEXT("user_id"));
                    User.Score = Obj->GetIntegerField(TEXT("score"));
                    Users.Add(User);

                    UE_LOG(LogTemp, Log, TEXT("User: %s, Score: %d"), *User.UserID, User.Score);
                }
            }

            // Users 배열을 게임 로직/UI에 사용
        }
    }
    else // 단일 객체
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            FDBData User;
            User.UserID = JsonObject->GetStringField(TEXT("user_id"));
            User.Score = JsonObject->GetIntegerField(TEXT("score"));

            UE_LOG(LogTemp, Log, TEXT("Single User: %s, Score: %d"), *User.UserID, User.Score);
        }
    }
}