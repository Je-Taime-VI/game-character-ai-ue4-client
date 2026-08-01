#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "Http.h"
#include "Input/Reply.h"
#include "Widgets/SCompoundWidget.h"

class AFeixunHUD;
class SEditableTextBox;
class SScrollBox;
class USoundBase;

enum class EFeixunMessageSide : uint8
{
    Character,
    Player,
    System
};

struct FFeixunMessage
{
    EFeixunMessageSide Side = EFeixunMessageSide::System;
    FString Speaker;
    FString Text;
};

struct FFeixunThread
{
    FString Title;
    FString ConversationId;
    TArray<FFeixunMessage> Messages;
};

struct FFeixunContact
{
    FString RoleName;
    FString Signature;
    FLinearColor AccentColor;
    FString AvatarFileName;
    TArray<FFeixunThread> Threads;
};

class SFeixunChatWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SFeixunChatWidget) {}
        SLATE_ARGUMENT(AFeixunHUD*, OwningHUD)
        SLATE_ARGUMENT(USoundBase*, ClickSound)
        SLATE_ARGUMENT(USoundBase*, SendSound)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void TogglePanel();
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
    AFeixunHUD* OwningHUD = nullptr;
    USoundBase* ClickSound = nullptr;
    USoundBase* SendSound = nullptr;

    FString ApiUrl = TEXT("http://127.0.0.1:8000/chat");
    FString ContactsUrl = TEXT("http://127.0.0.1:8000/roles");
    FString PlayerName = TEXT("Je'TaimeVI");
    int32 SelectedContactIndex = INDEX_NONE;
    int32 SelectedThreadIndex = INDEX_NONE;
    bool bWaitingReply = false;
    bool bPanelVisible = false;
    bool bContactsLoadedFromBackend = false;
    static const int32 MaxThreadsPerContact = 5;

    TArray<FFeixunContact> Contacts;
    mutable TMap<FString, TSharedPtr<FSlateDynamicImageBrush>> AvatarBrushes;

    TSharedPtr<SScrollBox> ContactListBox;
    TSharedPtr<SScrollBox> MessageListBox;
    TSharedPtr<SEditableTextBox> InputBox;

    void SeedContacts();
    void LoadContactsFromBackend();
    void HandleContactsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void RebuildAll();
    void RebuildContacts();
    void RebuildMessages();

    FReply OpenPanel();
    FReply ClosePanel();
    EVisibility GetPanelVisibility() const;
    EVisibility GetLauncherVisibility() const;
    FReply SelectContact(int32 ContactIndex);
    FReply NewThreadForSelectedContact();
    FReply SelectThread(int32 ThreadIndex);
    FReply SendCurrentInput();
    void OnInputCommitted(const FText& Text, ETextCommit::Type CommitType);

    void AddMessage(EFeixunMessageSide Side, const FString& Speaker, const FString& Text);
    TArray<FFeixunThread>* ActiveThreads();
    const TArray<FFeixunThread>* ActiveThreads() const;
    FFeixunThread* ActiveThread();
    const FFeixunThread* ActiveThread() const;
    TArray<FFeixunMessage>* ActiveMessages();
    const TArray<FFeixunMessage>* ActiveMessages() const;
    void SendChatRequest(const FString& Text);
    void HandleChatResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    TSharedRef<SWidget> BuildContactRow(int32 ContactIndex) const;
    TSharedRef<SWidget> BuildContactCard(int32 ContactIndex) const;
    TSharedRef<SWidget> BuildThreadRow(int32 ThreadIndex) const;
    TSharedRef<SWidget> BuildMessageRow(const FFeixunMessage& Message) const;
    const FFeixunContact* FindContactByName(const FString& Name) const;
    bool ApplyContactsPayload(const FString& Payload);
    FLinearColor AccentForRole(const FString& RoleName) const;
    TSharedPtr<FSlateDynamicImageBrush> GetAvatarBrush(const FString& Name, float Size) const;
    TSharedRef<SWidget> BuildAvatar(const FString& Name, const FLinearColor& Color, float Size) const;
    TSharedRef<SWidget> BuildHeader() const;
    TSharedRef<SWidget> BuildInputBar();

    FSlateColor DimText() const;
    FSlateColor MainText() const;
    FLinearColor ContactAccent(int32 ContactIndex) const;
    FString CurrentRoleName() const;
    FString CurrentSignature() const;
    FString CurrentConversationId() const;
    FString MakeThreadTitle(const FString& FirstMessage) const;
    void PlayClick() const;
    void PlaySend() const;
};


