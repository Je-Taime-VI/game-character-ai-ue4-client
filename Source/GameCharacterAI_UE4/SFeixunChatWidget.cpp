#include "SFeixunChatWidget.h"
#include "FeixunHUD.h"

#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Json.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Sound/SoundBase.h"
#include "InputCoreTypes.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FeixunChat"

namespace FeixunStyle
{
    static const FLinearColor Panel(0.82f, 0.87f, 0.88f, 0.92f);
    static const FLinearColor PanelLight(0.93f, 0.96f, 0.97f, 0.92f);
    static const FLinearColor PanelDark(0.20f, 0.23f, 0.24f, 0.68f);
    static const FLinearColor ThreadNormal(0.42f, 0.44f, 0.45f, 0.62f);
    static const FLinearColor ThreadDark(0.10f, 0.12f, 0.13f, 0.86f);
    static const FLinearColor Gold(0.92f, 0.80f, 0.28f, 1.0f);
    static const FLinearColor Ink(0.08f, 0.10f, 0.11f, 1.0f);
    static const FLinearColor Muted(0.48f, 0.52f, 0.53f, 1.0f);
    static const FString PlaceholderAvatarFile = TEXT("Placeholder.jpg");
    static const FString PlayerAvatarFile = TEXT("RoverMale.png");
}

void SFeixunChatWidget::Construct(const FArguments& InArgs)
{
    OwningHUD = InArgs._OwningHUD;
    ClickSound = InArgs._ClickSound;
    SendSound = InArgs._SendSound;
    SeedContacts();

    ChildSlot
    [
        SNew(SOverlay)
        + SOverlay::Slot()
        [
            SNew(SBorder)
            .Visibility(this, &SFeixunChatWidget::GetPanelVisibility)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.035f, 0.72f))
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SBox)
            .Visibility(this, &SFeixunChatWidget::GetPanelVisibility)
            .WidthOverride(1680.0f)
            .HeightOverride(900.0f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0.0f, 0.0f, 0.0f, 14.0f)
                [
                    SNew(SBorder)
                    .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                    .BorderBackgroundColor(FLinearColor(0.18f, 0.19f, 0.21f, 0.76f))
                    .Padding(FMargin(52.0f, 11.0f))
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("FeixunIcon", "◇"))
                            .ColorAndOpacity(FSlateColor(FLinearColor::White))
                            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 34))
                        ]
                        + SHorizontalBox::Slot().AutoWidth().Padding(28.0f, 0.0f).VAlign(VAlign_Center)
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("FeixunTitle", "飞讯"))
                            .ColorAndOpacity(FSlateColor(FLinearColor::White))
                            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 26))
                        ]
                        + SHorizontalBox::Slot().FillWidth(1.0f)
                        [SNew(SSpacer)]
                        + SHorizontalBox::Slot().AutoWidth().Padding(22.0f, 0.0f).VAlign(VAlign_Center)
                        [SNew(STextBlock).Text(LOCTEXT("SettingsIcon", "⚙")).ColorAndOpacity(FSlateColor(FLinearColor::White)).Font(FCoreStyle::GetDefaultFontStyle("Regular", 28))]
                        + SHorizontalBox::Slot().AutoWidth().Padding(22.0f, 0.0f).VAlign(VAlign_Center)
                        [SNew(STextBlock).Text(LOCTEXT("InfoIcon", "ⓘ")).ColorAndOpacity(FSlateColor(FLinearColor::White)).Font(FCoreStyle::GetDefaultFontStyle("Regular", 28))]
                        + SHorizontalBox::Slot().AutoWidth().Padding(22.0f, 0.0f).VAlign(VAlign_Center)
                        [
                            SNew(SButton)
                            .ButtonColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
                            .ContentPadding(FMargin(8.0f, 0.0f))
                            .OnClicked(this, &SFeixunChatWidget::ClosePanel)
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("CloseIcon", "×"))
                                .ColorAndOpacity(FSlateColor(FLinearColor::White))
                                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 28))
                            ]
                        ]
                    ]
                ]
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SBox)
                        .WidthOverride(460.0f)
                        [
                            SNew(SBorder)
                            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                            .BorderBackgroundColor(FeixunStyle::PanelDark)
                            .Padding(FMargin(26.0f, 20.0f))
                            [SAssignNew(ContactListBox, SScrollBox)]
                        ]
                    ]
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SBorder)
                        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                        .BorderBackgroundColor(FLinearColor(0.78f, 0.82f, 0.84f, 0.75f))
                        .Padding(FMargin(5.0f, 0.0f))
                        [SNew(SBox).WidthOverride(8.0f)]
                    ]
                    + SHorizontalBox::Slot()
                    .FillWidth(1.0f)
                    [
                        SNew(SBorder)
                        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                        .BorderBackgroundColor(FeixunStyle::Panel)
                        .Padding(FMargin(30.0f, 20.0f))
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot().AutoHeight()
                            [BuildHeader()]
                            + SVerticalBox::Slot().FillHeight(1.0f).Padding(0.0f, 16.0f, 0.0f, 10.0f)
                            [SAssignNew(MessageListBox, SScrollBox)]
                            + SVerticalBox::Slot().AutoHeight()
                            [BuildInputBar()]
                        ]
                    ]
                ]
            ]
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Top)
        .Padding(34.0f, 30.0f, 0.0f, 0.0f)
        [
            SNew(SButton)
            .Visibility(this, &SFeixunChatWidget::GetLauncherVisibility)
            .OnClicked(this, &SFeixunChatWidget::OpenPanel)
            .ButtonColorAndOpacity(FLinearColor(0.18f, 0.19f, 0.21f, 0.78f))
            .ContentPadding(FMargin(24.0f, 12.0f))
            [
                SNew(STextBlock)
                .Text(LOCTEXT("OpenFeixun", "◇  飞讯"))
                .ColorAndOpacity(FSlateColor(FLinearColor::White))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
            ]
        ]
    ];

    RebuildAll();
    LoadContactsFromBackend();
}

FReply SFeixunChatWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::I)
    {
        TogglePanel();
        return FReply::Handled();
    }
    return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SFeixunChatWidget::TogglePanel()
{
    bPanelVisible = !bPanelVisible;
    if (bPanelVisible && !bContactsLoadedFromBackend)
    {
        LoadContactsFromBackend();
    }
    PlayClick();
    if (bPanelVisible)
    {
        FSlateApplication::Get().SetKeyboardFocus(AsShared());
    }
}

FReply SFeixunChatWidget::OpenPanel()
{
    bPanelVisible = true;
    if (!bContactsLoadedFromBackend)
    {
        LoadContactsFromBackend();
    }
    PlayClick();
    return FReply::Handled();
}

FReply SFeixunChatWidget::ClosePanel()
{
    bPanelVisible = false;
    PlayClick();
    return FReply::Handled();
}

EVisibility SFeixunChatWidget::GetPanelVisibility() const
{
    return bPanelVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFeixunChatWidget::GetLauncherVisibility() const
{
    return bPanelVisible ? EVisibility::Collapsed : EVisibility::Visible;
}
void SFeixunChatWidget::SeedContacts()
{
    Contacts.Empty();
    Contacts.Add({TEXT("守岸人"), TEXT("点击开始飞讯对话。"), FLinearColor(0.55f, 0.76f, 1.0f, 1.0f), TEXT("ShouAnRen.png"), {}});
    Contacts.Add({TEXT("爱弥斯"), TEXT("只要抬头，那颗星总能找到我。"), FLinearColor(0.55f, 0.76f, 1.0f, 1.0f), TEXT("Aimis.png"), {}});
    Contacts.Add({TEXT("秧秧"), TEXT("点击开始飞讯对话。"), FLinearColor(0.62f, 0.68f, 0.72f, 1.0f), TEXT("Yangyang.png"), {}});
    Contacts.Add({TEXT("陆·赫斯"), TEXT("不给糖就捣蛋！:'-)"), FLinearColor(0.92f, 0.80f, 0.28f, 1.0f), TEXT("LuHesi.png"), {}});
}
void SFeixunChatWidget::LoadContactsFromBackend()
{
    FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(ContactsUrl);
    Request->SetVerb(TEXT("GET"));
    Request->OnProcessRequestComplete().BindRaw(this, &SFeixunChatWidget::HandleContactsResponse);
    Request->ProcessRequest();
}

void SFeixunChatWidget::HandleContactsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        return;
    }

    if (ApplyContactsPayload(Response->GetContentAsString()))
    {
        bContactsLoadedFromBackend = true;
        SelectedContactIndex = INDEX_NONE;
        SelectedThreadIndex = INDEX_NONE;
        RebuildAll();
    }
}

void SFeixunChatWidget::RebuildAll()
{
    RebuildContacts();
    RebuildMessages();
}

void SFeixunChatWidget::RebuildContacts()
{
    if (!ContactListBox.IsValid()) return;
    ContactListBox->ClearChildren();
    for (int32 Index = 0; Index < Contacts.Num(); ++Index)
    {
        ContactListBox->AddSlot().Padding(0.0f, 0.0f, 0.0f, 12.0f)[BuildContactRow(Index)];
    }
}

void SFeixunChatWidget::RebuildMessages()
{
    if (!MessageListBox.IsValid()) return;
    MessageListBox->ClearChildren();

    if (SelectedContactIndex == INDEX_NONE)
    {
        MessageListBox->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0.0f, 280.0f, 0.0f, 0.0f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("ChooseContact", "请选择飞讯联系人"))
            .ColorAndOpacity(DimText())
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
        ];
        return;
    }

    const TArray<FFeixunMessage>* CurrentMessages = ActiveMessages();
    if (SelectedThreadIndex == INDEX_NONE || !CurrentMessages)
    {
        MessageListBox->AddSlot().HAlign(HAlign_Center).Padding(0.0f, 250.0f, 0.0f, 0.0f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("ChooseThread", "选择左侧对话，或新建一个空白对话"))
            .ColorAndOpacity(DimText())
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
        ];
        return;
    }

    for (const FFeixunMessage& Message : *CurrentMessages)
    {
        MessageListBox->AddSlot().Padding(0.0f, 0.0f, 0.0f, 18.0f)[BuildMessageRow(Message)];
    }
    MessageListBox->ScrollToEnd();
}

TSharedRef<SWidget> SFeixunChatWidget::BuildContactRow(int32 ContactIndex) const
{
    TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
    Box->AddSlot().AutoHeight()[BuildContactCard(ContactIndex)];

    if (ContactIndex == SelectedContactIndex)
    {
        Box->AddSlot().AutoHeight().Padding(30.0f, 10.0f, 0.0f, 8.0f)
        [
            SNew(SButton)
            .OnClicked(const_cast<SFeixunChatWidget*>(this), &SFeixunChatWidget::NewThreadForSelectedContact)
            .ButtonColorAndOpacity(FeixunStyle::ThreadDark)
            .ContentPadding(FMargin(18.0f, 9.0f))
            [
                SNew(STextBlock)
                .Text(LOCTEXT("NewThread", "+ 新建空白对话"))
                .ColorAndOpacity(FSlateColor(FLinearColor::White))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
            ]
        ];

        const TArray<FFeixunThread>& Threads = Contacts[ContactIndex].Threads;
        for (int32 ThreadIndex = 0; ThreadIndex < Threads.Num(); ++ThreadIndex)
        {
            Box->AddSlot().AutoHeight().Padding(30.0f, 0.0f, 0.0f, 10.0f)[BuildThreadRow(ThreadIndex)];
        }
    }

    return Box;
}

TSharedRef<SWidget> SFeixunChatWidget::BuildContactCard(int32 ContactIndex) const
{
    const bool bSelected = ContactIndex == SelectedContactIndex;
    const FFeixunContact& Contact = Contacts[ContactIndex];
    return SNew(SButton)
        .OnClicked(const_cast<SFeixunChatWidget*>(this), &SFeixunChatWidget::SelectContact, ContactIndex)
        .ButtonColorAndOpacity(bSelected ? FeixunStyle::PanelLight : FLinearColor(0.55f, 0.58f, 0.58f, 0.52f))
        .ContentPadding(FMargin(26.0f, 12.0f))
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [BuildAvatar(Contact.RoleName, Contact.AccentColor, 66.0f)]
            + SHorizontalBox::Slot().AutoWidth().Padding(18.0f, 0.0f).VAlign(VAlign_Center)
            [SNew(STextBlock).Text(FText::FromString(TEXT("▌"))).ColorAndOpacity(FSlateColor(bSelected ? FeixunStyle::Gold : FLinearColor(0.85f, 0.88f, 0.88f, 0.75f))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))]
            + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
            [SNew(STextBlock).Text(FText::FromString(Contact.RoleName)).ColorAndOpacity(FSlateColor(bSelected ? FeixunStyle::Ink : FLinearColor::White)).Font(FCoreStyle::GetDefaultFontStyle("Bold", 26))]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [SNew(STextBlock).Text(FText::FromString(bSelected ? TEXT("▴") : TEXT("▾"))).ColorAndOpacity(DimText()).Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))]
        ];
}

TSharedRef<SWidget> SFeixunChatWidget::BuildThreadRow(int32 ThreadIndex) const
{
    const TArray<FFeixunThread>* Threads = ActiveThreads();
    const FFeixunThread& Thread = (*Threads)[ThreadIndex];
    const bool bSelected = ThreadIndex == SelectedThreadIndex;
    return SNew(SButton)
        .OnClicked(const_cast<SFeixunChatWidget*>(this), &SFeixunChatWidget::SelectThread, ThreadIndex)
        .ButtonColorAndOpacity(bSelected ? FeixunStyle::ThreadDark : FeixunStyle::ThreadNormal)
        .ContentPadding(FMargin(18.0f, 11.0f))
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [SNew(STextBlock).Text(FText::FromString(TEXT("✓"))).ColorAndOpacity(FSlateColor(FLinearColor::White)).Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))]
            + SHorizontalBox::Slot().AutoWidth().Padding(14.0f, 0.0f).VAlign(VAlign_Center)
            [SNew(STextBlock).Text(FText::FromString(TEXT("▌"))).ColorAndOpacity(FSlateColor(bSelected ? FeixunStyle::Gold : FLinearColor(0.78f, 0.80f, 0.80f, 0.58f))).Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))]
            + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
            [SNew(STextBlock).Text(FText::FromString(Thread.Title)).ColorAndOpacity(FSlateColor(FLinearColor::White)).Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))]
        ];
}

TSharedRef<SWidget> SFeixunChatWidget::BuildMessageRow(const FFeixunMessage& Message) const
{
    const bool bPlayer = Message.Side == EFeixunMessageSide::Player;
    const FLinearColor BubbleColor = bPlayer ? FLinearColor(0.05f, 0.06f, 0.07f, 0.92f) : FLinearColor(0.94f, 0.97f, 0.98f, 0.96f);
    const FSlateColor TextColor = FSlateColor(bPlayer ? FLinearColor::White : FeixunStyle::Ink);
    const FLinearColor AvatarColor = bPlayer ? FLinearColor(0.16f, 0.18f, 0.22f, 1.0f) : ContactAccent(SelectedContactIndex);

    TSharedRef<SWidget> Bubble = SNew(SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(BubbleColor)
        .Padding(FMargin(22.0f, 14.0f))
        [
            SNew(STextBlock)
            .Text(FText::FromString(Message.Text))
            .AutoWrapText(true)
            .WrapTextAt(610.0f)
            .ColorAndOpacity(TextColor)
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
        ];

    if (Message.Side == EFeixunMessageSide::System)
    {
        return SNew(STextBlock).Text(FText::FromString(Message.Text)).ColorAndOpacity(DimText()).Justification(ETextJustify::Center);
    }

    if (bPlayer)
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.0f)[SNew(SSpacer)]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
                [SNew(STextBlock).Text(FText::FromString(PlayerName)).ColorAndOpacity(DimText()).Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))]
                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 12.0f, 0.0f)
                [Bubble]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)
            [BuildAvatar(PlayerName, AvatarColor, 74.0f)];
    }

    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)
        [BuildAvatar(Message.Speaker, AvatarColor, 74.0f)]
        + SHorizontalBox::Slot().AutoWidth().Padding(14.0f, 0.0f, 0.0f, 0.0f).VAlign(VAlign_Top)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [SNew(STextBlock).Text(FText::FromString(Message.Speaker)).ColorAndOpacity(DimText()).Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
            [Bubble]
        ];
}

const FFeixunContact* SFeixunChatWidget::FindContactByName(const FString& Name) const
{
    for (const FFeixunContact& Contact : Contacts)
    {
        if (Contact.RoleName == Name)
        {
            return &Contact;
        }
    }
    return nullptr;
}

bool SFeixunChatWidget::ApplyContactsPayload(const FString& Payload)
{
    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Payload);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* Items = nullptr;
    if (!Root->TryGetArrayField(TEXT("items"), Items))
    {
        return false;
    }

    TArray<FFeixunContact> LoadedContacts;
    for (const TSharedPtr<FJsonValue>& ItemValue : *Items)
    {
        const TSharedPtr<FJsonObject> Item = ItemValue.IsValid() ? ItemValue->AsObject() : nullptr;
        if (!Item.IsValid())
        {
            continue;
        }

        FString RoleName;
        if (!Item->TryGetStringField(TEXT("role_name"), RoleName) || RoleName.IsEmpty())
        {
            continue;
        }

        FString Signature;
        if (!Item->TryGetStringField(TEXT("signature"), Signature) || Signature.IsEmpty())
        {
            Signature = TEXT("点击开始飞讯对话。");
        }

        FString AvatarFileName;
        if (!Item->TryGetStringField(TEXT("avatar_file"), AvatarFileName) || AvatarFileName.IsEmpty())
        {
            AvatarFileName = FeixunStyle::PlaceholderAvatarFile;
        }

        LoadedContacts.Add({RoleName, Signature, AccentForRole(RoleName), AvatarFileName, {}});
    }

    if (LoadedContacts.Num() == 0)
    {
        return false;
    }

    Contacts = MoveTemp(LoadedContacts);
    return true;
}

FLinearColor SFeixunChatWidget::AccentForRole(const FString& RoleName) const
{
    if (RoleName == TEXT("陆·赫斯"))
    {
        return FLinearColor(0.92f, 0.80f, 0.28f, 1.0f);
    }
    if (RoleName == TEXT("爱弥斯"))
    {
        return FLinearColor(0.55f, 0.76f, 1.0f, 1.0f);
    }
    return FLinearColor(0.62f, 0.68f, 0.72f, 1.0f);
}

TSharedPtr<FSlateDynamicImageBrush> SFeixunChatWidget::GetAvatarBrush(const FString& Name, float Size) const
{
    const FFeixunContact* Contact = FindContactByName(Name);
    FString AvatarFileName = Contact ? Contact->AvatarFileName : TEXT("");
    if (AvatarFileName.IsEmpty())
    {
        AvatarFileName = Name == PlayerName ? FeixunStyle::PlayerAvatarFile : FeixunStyle::PlaceholderAvatarFile;
    }

    FString AvatarPath = FPaths::ProjectContentDir() + TEXT("Feixun/Avatars/") + AvatarFileName;
    if (!FPaths::FileExists(AvatarPath) && AvatarFileName != FeixunStyle::PlaceholderAvatarFile)
    {
        AvatarFileName = FeixunStyle::PlaceholderAvatarFile;
        AvatarPath = FPaths::ProjectContentDir() + TEXT("Feixun/Avatars/") + AvatarFileName;
    }
    if (!FPaths::FileExists(AvatarPath))
    {
        return nullptr;
    }

    const int32 RoundedSize = FMath::RoundToInt(Size);
    const FString BrushKey = FString::Printf(TEXT("%s:%d"), *AvatarFileName, RoundedSize);
    if (const TSharedPtr<FSlateDynamicImageBrush>* ExistingBrush = AvatarBrushes.Find(BrushKey))
    {
        return *ExistingBrush;
    }

    TSharedPtr<FSlateDynamicImageBrush> Brush = MakeShared<FSlateDynamicImageBrush>(
        FName(*AvatarPath),
        FVector2D(Size, Size)
    );
    AvatarBrushes.Add(BrushKey, Brush);
    return Brush;
}
TSharedRef<SWidget> SFeixunChatWidget::BuildAvatar(const FString& Name, const FLinearColor& Color, float Size) const
{
    if (TSharedPtr<FSlateDynamicImageBrush> AvatarBrush = GetAvatarBrush(Name, Size))
    {
        return SNew(SBox)
            .WidthOverride(Size)
            .HeightOverride(Size)
            [
                SNew(SImage)
                .Image(AvatarBrush.Get())
            ];
    }

    FString Initial = Name.Len() > 0 ? Name.Left(1) : TEXT("?");
    const int32 InitialFontSize = static_cast<int32>(Size * 0.36f);
    return SNew(SBox)
        .WidthOverride(Size)
        .HeightOverride(Size)
        [
            SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(Color.CopyWithNewOpacity(0.92f))
            .Padding(FMargin(0.0f))
            [
                SNew(STextBlock)
                .Text(FText::FromString(Initial))
                .Justification(ETextJustify::Center)
                .ColorAndOpacity(FSlateColor(FLinearColor::White))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", InitialFontSize))
            ]
        ];
}
TSharedRef<SWidget> SFeixunChatWidget::BuildHeader() const
{
    return SNew(SBorder)
        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor(0.71f, 0.75f, 0.76f, 0.58f))
        .Padding(FMargin(38.0f, 20.0f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(CurrentRoleName()); }).ColorAndOpacity(MainText()).Font(FCoreStyle::GetDefaultFontStyle("Bold", 30))]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f, 0.0f, 0.0f)
            [SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(CurrentSignature()); }).ColorAndOpacity(DimText()).Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))]
        ];
}

TSharedRef<SWidget> SFeixunChatWidget::BuildInputBar()
{
    return SNew(SHorizontalBox)
        .Visibility_Lambda([this]() { return SelectedContactIndex != INDEX_NONE && SelectedThreadIndex != INDEX_NONE ? EVisibility::Visible : EVisibility::Collapsed; })
        + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
        [
            SAssignNew(InputBox, SEditableTextBox)
            .HintText(LOCTEXT("InputHint", "输入飞讯内容，按 Enter 发送"))
            .IsEnabled_Lambda([this]() { return SelectedContactIndex != INDEX_NONE && SelectedThreadIndex != INDEX_NONE && !bWaitingReply; })
            .OnTextCommitted(this, &SFeixunChatWidget::OnInputCommitted)
            .Font(FCoreStyle::GetDefaultFontStyle("Regular", 20))
        ]
        + SHorizontalBox::Slot().AutoWidth().Padding(12.0f, 0.0f, 0.0f, 0.0f).VAlign(VAlign_Center)
        [
            SNew(SButton)
            .Text(LOCTEXT("Send", "发送"))
            .OnClicked(this, &SFeixunChatWidget::SendCurrentInput)
            .IsEnabled_Lambda([this]() { return SelectedContactIndex != INDEX_NONE && SelectedThreadIndex != INDEX_NONE && !bWaitingReply; })
        ];
}

FReply SFeixunChatWidget::SelectContact(int32 ContactIndex)
{
    PlayClick();
    SelectedContactIndex = ContactIndex;
    SelectedThreadIndex = INDEX_NONE;
    RebuildAll();
    return FReply::Handled();
}

FReply SFeixunChatWidget::NewThreadForSelectedContact()
{
    TArray<FFeixunThread>* Threads = ActiveThreads();
    if (!Threads)
    {
        return FReply::Handled();
    }
    PlayClick();
    FFeixunThread Thread;
    Thread.Title = TEXT("新的空白对话");
    Thread.ConversationId = FString::Printf(TEXT("ue4_%s_%s"), *Contacts[SelectedContactIndex].RoleName, *FGuid::NewGuid().ToString(EGuidFormats::Digits));
    Threads->Add(Thread);
    if (Threads->Num() > MaxThreadsPerContact)
    {
        Threads->RemoveAt(0);
    }
    SelectedThreadIndex = Threads->Num() - 1;
    RebuildAll();
    return FReply::Handled();
}

FReply SFeixunChatWidget::SelectThread(int32 ThreadIndex)
{
    PlayClick();
    SelectedThreadIndex = ThreadIndex;
    RebuildAll();
    return FReply::Handled();
}

FReply SFeixunChatWidget::SendCurrentInput()
{
    if (!InputBox.IsValid() || SelectedContactIndex == INDEX_NONE || SelectedThreadIndex == INDEX_NONE || bWaitingReply)
    {
        return FReply::Handled();
    }

    const FString Text = InputBox->GetText().ToString().TrimStartAndEnd();
    if (Text.IsEmpty())
    {
        return FReply::Handled();
    }

    PlaySend();
    InputBox->SetText(FText::GetEmpty());
    AddMessage(EFeixunMessageSide::Player, PlayerName, Text);
    if (FFeixunThread* Thread = ActiveThread())
    {
        if (Thread->Title == TEXT("新的空白对话"))
        {
            Thread->Title = MakeThreadTitle(Text);
            RebuildContacts();
        }
    }
    SendChatRequest(Text);
    RebuildMessages();
    return FReply::Handled();
}

void SFeixunChatWidget::OnInputCommitted(const FText& Text, ETextCommit::Type CommitType)
{
    if (CommitType == ETextCommit::OnEnter)
    {
        SendCurrentInput();
    }
}

void SFeixunChatWidget::AddMessage(EFeixunMessageSide Side, const FString& Speaker, const FString& Text)
{
    if (TArray<FFeixunMessage>* CurrentMessages = ActiveMessages())
    {
        CurrentMessages->Add({Side, Speaker, Text});
    }
}

TArray<FFeixunThread>* SFeixunChatWidget::ActiveThreads()
{
    return Contacts.IsValidIndex(SelectedContactIndex) ? &Contacts[SelectedContactIndex].Threads : nullptr;
}

const TArray<FFeixunThread>* SFeixunChatWidget::ActiveThreads() const
{
    return Contacts.IsValidIndex(SelectedContactIndex) ? &Contacts[SelectedContactIndex].Threads : nullptr;
}

FFeixunThread* SFeixunChatWidget::ActiveThread()
{
    TArray<FFeixunThread>* Threads = ActiveThreads();
    return Threads && Threads->IsValidIndex(SelectedThreadIndex) ? &(*Threads)[SelectedThreadIndex] : nullptr;
}

const FFeixunThread* SFeixunChatWidget::ActiveThread() const
{
    const TArray<FFeixunThread>* Threads = ActiveThreads();
    return Threads && Threads->IsValidIndex(SelectedThreadIndex) ? &(*Threads)[SelectedThreadIndex] : nullptr;
}

TArray<FFeixunMessage>* SFeixunChatWidget::ActiveMessages()
{
    FFeixunThread* Thread = ActiveThread();
    return Thread ? &Thread->Messages : nullptr;
}

const TArray<FFeixunMessage>* SFeixunChatWidget::ActiveMessages() const
{
    const FFeixunThread* Thread = ActiveThread();
    return Thread ? &Thread->Messages : nullptr;
}

void SFeixunChatWidget::SendChatRequest(const FString& Text)
{
    bWaitingReply = true;
    AddMessage(EFeixunMessageSide::System, TEXT("系统"), TEXT("对方正在输入……"));

    TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
    Payload->SetStringField(TEXT("role_name"), CurrentRoleName());
    Payload->SetStringField(TEXT("message"), Text);
    Payload->SetStringField(TEXT("conversation_id"), CurrentConversationId());
    Payload->SetBoolField(TEXT("remember"), true);
    Payload->SetNumberField(TEXT("top_k_lines"), 8);
    Payload->SetNumberField(TEXT("top_k_flat"), 5);

    FString Body;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
    FJsonSerializer::Serialize(Payload, Writer);

    FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(ApiUrl);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
    Request->SetContentAsString(Body);
    Request->OnProcessRequestComplete().BindRaw(this, &SFeixunChatWidget::HandleChatResponse);
    Request->ProcessRequest();
}

void SFeixunChatWidget::HandleChatResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    bWaitingReply = false;
    if (TArray<FFeixunMessage>* CurrentMessages = ActiveMessages())
    {
        if (CurrentMessages->Num() > 0 && CurrentMessages->Last().Side == EFeixunMessageSide::System)
        {
            CurrentMessages->RemoveAt(CurrentMessages->Num() - 1);
        }
    }

    FString Reply = TEXT("连接失败。请确认 FastAPI 后端和 Ollama 已启动。");
    if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        TSharedPtr<FJsonObject> Root;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
        {
            Reply = Root->GetStringField(TEXT("reply"));
        }
    }

    AddMessage(EFeixunMessageSide::Character, CurrentRoleName(), Reply);
    RebuildMessages();
}

FSlateColor SFeixunChatWidget::DimText() const
{
    return FSlateColor(FeixunStyle::Muted);
}

FSlateColor SFeixunChatWidget::MainText() const
{
    return FSlateColor(FeixunStyle::Ink);
}

FLinearColor SFeixunChatWidget::ContactAccent(int32 ContactIndex) const
{
    return Contacts.IsValidIndex(ContactIndex) ? Contacts[ContactIndex].AccentColor : FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);
}

FString SFeixunChatWidget::CurrentRoleName() const
{
    return Contacts.IsValidIndex(SelectedContactIndex) ? Contacts[SelectedContactIndex].RoleName : TEXT("");
}

FString SFeixunChatWidget::CurrentSignature() const
{
    return Contacts.IsValidIndex(SelectedContactIndex) ? Contacts[SelectedContactIndex].Signature : TEXT("");
}

FString SFeixunChatWidget::CurrentConversationId() const
{
    const FFeixunThread* Thread = ActiveThread();
    return Thread ? Thread->ConversationId : TEXT("ue4_default");
}

FString SFeixunChatWidget::MakeThreadTitle(const FString& FirstMessage) const
{
    if (FirstMessage.Len() <= 16)
    {
        return FirstMessage;
    }
    return FirstMessage.Left(16) + TEXT("…");
}

void SFeixunChatWidget::PlayClick() const
{
    if (ClickSound && OwningHUD)
    {
        UGameplayStatics::PlaySound2D(OwningHUD, ClickSound);
    }
}

void SFeixunChatWidget::PlaySend() const
{
    if (SendSound && OwningHUD)
    {
        UGameplayStatics::PlaySound2D(OwningHUD, SendSound);
    }
}

#undef LOCTEXT_NAMESPACE





