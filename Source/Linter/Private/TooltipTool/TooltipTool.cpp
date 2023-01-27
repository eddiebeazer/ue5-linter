// Copyright 2016 Gamemakin LLC. All Rights Reserved.

#include "TooltipEditor/TooltipTool.h"

#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SHeader.h"
#include "Widgets/Images/SImage.h"
#include "Framework/Text/TextLayout.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Input/SButton.h"
#include "Types/SlateEnums.h"
#include "Editor.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "FileHelpers.h"

#define LOCTEXT_NAMESPACE "LinterTooltipTool"

FTooltipTool::FTooltipTool(const TArray<FAssetData> Assets)
{
	for (auto Asset : Assets)
	{
		if (Asset.GetClass()->IsChildOf(UBlueprint::StaticClass()))
		{
			BlueprintsInternal.Push(Asset);
		}
	}

	for (int32 i = 0; i < BlueprintsInternal.Num(); ++i)
	{
		Blueprints.Push(TSharedPtr<FAssetData>(&BlueprintsInternal[i]));
	}

	if (FSlateApplication::IsInitialized())
	{
		DialogWindow = SNew(SWindow)
			.Title(LOCTEXT("TooltipToolDlgTitle", "Blueprint Member Tooltip Tool"))
			.SupportsMinimize(false).SupportsMaximize(false)
			.SaneWindowPlacement(true)
			.AutoCenter(EAutoCenter::PreferredWorkArea)
			.MinWidth(400.0f)
			.MaxWidth(400.0f)
			.SizingRule(ESizingRule::Autosized);

		TSharedPtr<SBorder> DialogWrapper =
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(4.0f)
			[
				SAssignNew(DialogWidget, STooltipTool)
				.ParentWindow(DialogWindow)
				.Blueprints(Blueprints)
			];

		DialogWindow->SetContent(DialogWrapper.ToSharedRef());
	}
}

FTooltipTool::EResult FTooltipTool::ShowModal()
{
	//Show Dialog
	GEditor->EditorAddModalWindow(DialogWindow.ToSharedRef());
	EResult UserResponse = (EResult)DialogWidget->GetUserResponse();

	if (UserResponse == EResult::Confirm)
	{


	}
	return UserResponse;
}

void STooltipTool::Construct(const FArguments& InArgs)
{
	UserResponse = FTooltipTool::Cancel;
	ParentWindow = InArgs._ParentWindow.Get();
	Blueprints = InArgs._Blueprints;
	check(Blueprints.IsSet() && Blueprints.Get().Num() > 0);

	this->ChildSlot[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f, 8.0f, 0.0f)
		[
			SNew(SHeader)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TooltipToolBlueprintHeader", "Blueprint"))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f, 8.0f, 4.0f)
		[
			SAssignNew(BlueprintComboBox, SComboBox<TSharedPtr<FAssetData>>)
			.OptionsSource(&Blueprints.Get())
			.InitiallySelectedItem(Blueprints.Get()[0])
			.OnGenerateWidget_Lambda(
			[](TSharedPtr<FAssetData> Item)
			{
				return SNew(STextBlock)
					.Text(FText::FromName(Item->AssetName))
					.ToolTipText(FText::FromString(Item->GetFullName()));
			})
			.OnSelectionChanged_Lambda(
				[&](TSharedPtr<FAssetData> Item, ESelectInfo::Type SelectInfo)
				{
					VariableTooltipEditableTextBox->SetEnabled(false);
					RebuildMemberList();
				})
			[
				SNew(STextBlock)
				.Text(this, &STooltipTool::GetSelectedBlueprintText)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f, 8.0f, 4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.OnClicked_Lambda(
					[&] {
						int32 Index = Blueprints.Get().Find(BlueprintComboBox->GetSelectedItem());
						Index = (Blueprints.Get().Num() + Index - 1) % Blueprints.Get().Num();
						BlueprintComboBox->SetSelectedItem(Blueprints.Get()[Index]);
						return FReply::Handled();
					})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TooltipToolBlueprintsPrevious", "Previous"))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.OnClicked_Lambda(
					[&] {
						int32 Index = Blueprints.Get().Find(BlueprintComboBox->GetSelectedItem());
						Index = (Index + 1) % Blueprints.Get().Num();
						BlueprintComboBox->SetSelectedItem(Blueprints.Get()[Index]);
						return FReply::Handled();
					})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TooltipToolBlueprintsNext", "Next"))
				]
			]

		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f, 8.0f, 4.0f)
		[
			SNew(SButton)
			.OnClicked_Lambda(
				[&]{
					// Save package here if SCC is enabled because the user can use SCC to revert a change
					TArray<UPackage*> OutermostPackagesToSave;
					for (auto Asset : Blueprints.Get())
					{
						OutermostPackagesToSave.Add(Asset->GetPackage());
					}
					FEditorFileUtils::PromptForCheckoutAndSave(OutermostPackagesToSave, true, false);
					return FReply::Handled();
				})
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TooltipToolBlueprintsSaveButtonLabel", "Save All Blueprints"))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f, 8.0f, 4.0f)
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(true)
			.AreaTitle(LOCTEXT("TooltipToolVariablesExpandableTitle", "Variables"))
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f, 4.0f, 8.0f, 4.0f)
				[
					SNew(SBox)
					.HeightOverride(100.0f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
						.Padding(0.0f)
						[
							SNew(SScrollBox)
							.ScrollBarAlwaysVisible(true)
							+ SScrollBox::Slot()
							[
								SAssignNew(MemberListView, SListView<TSharedPtr<FBPVariableDescription>>)
								.SelectionMode(ESelectionMode::Single)
								.ListItemsSource(&Members)
								.OnGenerateRow_Lambda(
								[](TSharedPtr<FBPVariableDescription> Item, const TSharedRef<STableViewBase>& OwnerTable)
								{
									return SNew(STableRow<TSharedPtr<FBPVariableDescription>>, OwnerTable)
										[
											SNew(SHorizontalBox)
											+ SHorizontalBox::Slot()
											.AutoWidth()
											[
												SNew(SImage)
												.Image(FEditorStyle::GetBrush(TEXT("Icons.Error")))
												.Visibility_Lambda([Item] {return Item.IsValid() ? (Item->HasMetaData(FBlueprintMetadata::MD_Tooltip) && Item->GetMetaData(FBlueprintMetadata::MD_Tooltip).Len() > 0 ? EVisibility::Collapsed : EVisibility::HitTestInvisible) : EVisibility::Collapsed; })
											]
											+ SHorizontalBox::Slot()
											.AutoWidth()
											[
												SNew(STextBlock)
												.Text(FText::FromString(Item->FriendlyName))
											]
											+ SHorizontalBox::Slot()
											.FillWidth(1.0f)
											[
												SNew(STextBlock)
												.Text(UEdGraphSchema_K2::TypeToText(Item->VarType))
												.Justification(ETextJustify::Right)
											]
										];
								})
							.OnSelectionChanged_Lambda(
								[&](TSharedPtr<FBPVariableDescription> Item, ESelectInfo::Type SelectInfo)
								{
									if (!Item.IsValid())
									{
										VariableTooltipEditableTextBox->SetEnabled(false);
										VariableTooltipEditableTextBox->SetText(FText::GetEmpty());
										return;
									}

									VariableTooltipEditableTextBox->SetEnabled(true);
									if (Item->HasMetaData(FBlueprintMetadata::MD_Tooltip))
									{
										VariableTooltipEditableTextBox->SetText(FText::FromString(Item->GetMetaData(FBlueprintMetadata::MD_Tooltip)));
									}
									else
									{
										VariableTooltipEditableTextBox->SetText(FText::GetEmpty());
									}
								})
							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f, 4.0f, 8.0f, 4.0f)
				[
					SNew(SHeader)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("TooltipToolTooltipHeader", "Tooltip"))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f, 4.0f, 8.0f, 4.0f)
				[
					SNew(SBox)
					.HeightOverride(100.0f)
					[
						SAssignNew(VariableTooltipEditableTextBox, SMultiLineEditableTextBox)
						.AutoWrapText(true)
						.IsEnabled(false)
						.OnTextChanged_Lambda(
							[&](const FText& NewText)
							{
								if (CommitOnTextChangeCheckBox->IsChecked())
								{
									UpdateVariableTooltipText(NewText);
								}
							})
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f, 4.0f, 8.0f, 4.0f)
				[
					SAssignNew(CommitTextButton, SButton)
					.OnClicked_Lambda([&] { /**UpdateVariableTooltipText(TooltipEditableTextBox->GetText());**/ return FReply::Handled(); })
					.Visibility_Lambda([&] {return CommitOnTextChangeCheckBox->IsChecked() ? EVisibility::Collapsed : EVisibility::Visible; })
					[
						SNew(STextBlock)
						.Text(LOCTEXT("TooltipToolCommitTextButtonLabel", "Commit Text"))
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f, 8.0f, 4.0f)
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(true)
			.AreaTitle(LOCTEXT("TooltipToolFunctionExpandableTitle", "Functions"))
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f, 4.0f, 8.0f, 4.0f)
				[
					SNew(SBox)
					.HeightOverride(100.0f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
						.Padding(0.0f)
						[
							SNew(SScrollBox)
							.ScrollBarAlwaysVisible(true)
							+ SScrollBox::Slot()
							[
								SAssignNew(FunctionListView, SListView<TSharedPtr<FBPFunctionPointers>>)
								.SelectionMode(ESelectionMode::Single)
								.ListItemsSource(&FunctionPointers)
								.OnGenerateRow_Lambda(
								[](TSharedPtr<FBPFunctionPointers> Item, const TSharedRef<STableViewBase>& OwnerTable)
								{
									return SNew(STableRow<TSharedPtr<FBPFunctionPointers>>, OwnerTable)
										[
											SNew(SHorizontalBox)
											+ SHorizontalBox::Slot()
											.AutoWidth()
											[
												SNew(SImage)
												.Image(FEditorStyle::GetBrush(TEXT("Icons.Error")))
												.Visibility_Lambda([Item] {return (Item.IsValid() && Item->FunctionEntryNode != nullptr&&  !Item->FunctionEntryNode->MetaData.ToolTip.IsEmptyOrWhitespace()) ? EVisibility::Collapsed : EVisibility::Visible; })
											]
											+ SHorizontalBox::Slot()
											.AutoWidth()
											[
												SNew(STextBlock)
												.Text(FText::FromName(Item->FunctionName))
											]
										];
								})
							.OnSelectionChanged_Lambda(
								[&](TSharedPtr<FBPFunctionPointers> Item, ESelectInfo::Type SelectInfo)
								{
									FunctionArgumentDescriptions.Empty();
									FunctionOutputDescriptions.Empty();

									if (!Item.IsValid())
									{
										FunctionArgumentListView->RebuildList();
										FunctionOutputListView->RebuildList();

										FunctionDescriptionTooltipBox->SetEnabled(false);
										FunctionArgumentListView->SetEnabled(false);
										FunctionOutputListView->SetEnabled(false);
										FunctionDescriptionTooltipBox->SetText(FText::GetEmpty());

										return;
									}

									check(Item->FunctionEntryNode);
									TArray<UEdGraphPin*> InputPins = Item->FunctionEntryNode->GetAllPins();
									for (UEdGraphPin* Pin : InputPins)
									{
										if (Pin->Direction == EEdGraphPinDirection::EGPD_Output)
										{	
											if (Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
											{
												FunctionArgumentDescriptions.Add(MakeShared<FBPFunctionArgumentDescription>(FText::FromString(Pin->GetName()), FText::GetEmpty(), UEdGraphSchema_K2::TypeToText(Pin->PinType)));
											}												
										} 
									}

									if (Item->FunctionResultNode != nullptr)
									{
										TArray<UEdGraphPin*> OutputPins = Item->FunctionResultNode->GetAllPins();
										for (UEdGraphPin* Pin : OutputPins)
										{
											if (Pin->Direction == EEdGraphPinDirection::EGPD_Input)
											{
												if (Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
												{
													FunctionOutputDescriptions.Add(MakeShared<FBPFunctionArgumentDescription>(FText::FromString(Pin->GetName()), FText::GetEmpty(), UEdGraphSchema_K2::TypeToText(Pin->PinType)));
												}
											}
										}
									}

									FunctionDescriptionTooltipBox->SetEnabled(true);
									FunctionArgumentListView->SetEnabled(true);
									FunctionOutputListView->SetEnabled(true);

									FText FunctionDescription;
									FText ReturnText;

									if (!FTooltipStringHelper::ParseFunctionRawTooltip(Item->FunctionEntryNode->MetaData.ToolTip.ToString(), CurrentFunctionDescription, FunctionArgumentDescriptions, FunctionOutputDescriptions, ReturnText))
									{
										CurrentFunctionDescription = FText::GetEmpty();
									}

									if (!ReturnText.IsEmptyOrWhitespace() && FunctionOutputDescriptions.Num() > 0)
									{
										FunctionOutputDescriptions[FunctionOutputDescriptions.Num() - 1]->Tooltip = ReturnText;
									}

									FunctionArgumentListView->RebuildList();
									FunctionOutputListView->RebuildList();

									FunctionDescriptionTooltipBox->SetText(CurrentFunctionDescription);
								
								})
							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f, 4.0f, 8.0f, 4.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHeader)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("TooltipToolFunctionDesc", "Description"))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(60.0f)
						[
							SAssignNew(FunctionDescriptionTooltipBox, SMultiLineEditableTextBox)
							.AutoWrapText(true)
							.IsEnabled(false)
							.OnTextChanged_Lambda(
								[&](const FText& NewText)
								{
									if (CommitOnTextChangeCheckBox->IsChecked())
									{
										if (!NewText.EqualTo(CurrentFunctionDescription))
										{
											UpdateCurrentFunctionTooltipText();
										}
										
									}
								})
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHeader)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("TooltipToolFunctionInputs", "Inputs"))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.MinDesiredHeight(20.0f)
						.MaxDesiredHeight(100.0f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
							.Padding(0.0f)
							[
								SNew(SScrollBox)
								.ScrollBarAlwaysVisible(true)
								+ SScrollBox::Slot()
								[
									SAssignNew(FunctionArgumentListView, SListView<TSharedPtr<FBPFunctionArgumentDescription>>)
									.SelectionMode(ESelectionMode::None)
									.ListItemsSource(&FunctionArgumentDescriptions)
									.OnGenerateRow_Lambda(
									[&](TSharedPtr<FBPFunctionArgumentDescription> Item, const TSharedRef<STableViewBase>& OwnerTable)
									{
										return SNew(STableRow<TSharedPtr<FBPFunctionArgumentDescription>>, OwnerTable)
											[
												SNew(SVerticalBox)
												+ SVerticalBox::Slot()
												[
													SNew(SHorizontalBox)
													+ SHorizontalBox::Slot()
													.AutoWidth()
													[
														SNew(SImage)
														.Image(FEditorStyle::GetBrush(TEXT("Icons.Error")))
														.Visibility_Lambda([Item] {return (Item.IsValid() && !Item->Tooltip.IsEmptyOrWhitespace()) ? EVisibility::Collapsed : EVisibility::Visible; })
													]
													+ SHorizontalBox::Slot()
													.AutoWidth()
													[
														SNew(STextBlock)
														.Text(FText::FromString(FString::Printf(TEXT("%s (%s)"), *Item->ArgumentName.ToString(), *Item->ArgumentType.ToString())))
													]
												]
												+ SVerticalBox::Slot()
												.Padding(0.0f, 4.0f, 0.0f, 4.0f)
												[
													SNew(SEditableTextBox)
													.Text(Item->Tooltip)
													.OnTextChanged_Lambda(
														[&](const FText& NewText)
														{
															if (CommitOnTextChangeCheckBox->IsChecked())
															{
																UpdateCurrentFunctionTooltipText();
															}
														})
												]
											];
									})
								]
							]
						]
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHeader)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("TooltipToolFunctionOutputs", "Outputs"))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.MinDesiredHeight(20.0f)
						.MaxDesiredHeight(100.0f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
							.Padding(0.0f)
							[
								SNew(SScrollBox)
								.ScrollBarAlwaysVisible(true)
								+ SScrollBox::Slot()
								[
									SAssignNew(FunctionOutputListView, SListView<TSharedPtr<FBPFunctionArgumentDescription>>)
									.SelectionMode(ESelectionMode::None)
									.ListItemsSource(&FunctionOutputDescriptions)
									.OnGenerateRow_Lambda(
									[&](TSharedPtr<FBPFunctionArgumentDescription> Item, const TSharedRef<STableViewBase>& OwnerTable)
									{
										return SNew(STableRow<TSharedPtr<FBPFunctionArgumentDescription>>, OwnerTable)
											[
												SNew(SVerticalBox)
												+ SVerticalBox::Slot()
												[
													SNew(SHorizontalBox)
													+ SHorizontalBox::Slot()
													.AutoWidth()
													[
														SNew(SImage)
														.Image(FEditorStyle::GetBrush(TEXT("Icons.Error")))
														.Visibility_Lambda([Item] {return (Item.IsValid() && !Item->Tooltip.IsEmptyOrWhitespace()) ? EVisibility::Collapsed : EVisibility::Visible; })
													]
													+ SHorizontalBox::Slot()
													.AutoWidth()
													[
														SNew(STextBlock)
														.Text(FText::FromString(FString::Printf(TEXT("%s (%s)"), *Item->ArgumentName.ToString(), *Item->ArgumentType.ToString())))
													]
												]
												+ SVerticalBox::Slot()
												.Padding(0.0f, 4.0f, 0.0f, 4.0f)
												[
													SNew(SEditableTextBox)
													.Text(Item->Tooltip)
													.OnTextChanged_Lambda(
														[&](const FText& NewText)
														{
															if (CommitOnTextChangeCheckBox->IsChecked())
															{
																UpdateCurrentFunctionTooltipText();
															}
														})
												]
											];
									})
								]
							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f, 4.0f, 8.0f, 4.0f)
				[
					SAssignNew(CommitTextButton, SButton)
					.OnClicked_Lambda([&] { UpdateVariableTooltipText(VariableTooltipEditableTextBox->GetText()); return FReply::Handled(); })
					.Visibility_Lambda([&] {return CommitOnTextChangeCheckBox->IsChecked() ? EVisibility::Collapsed : EVisibility::Visible; })
					[
						SNew(STextBlock)
						.Text(LOCTEXT("TooltipToolCommitTextButtonLabel", "Commit Text"))
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f, 8.0f, 4.0f)
		[
			SNew(SSeparator)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f, 8.0f, 0.0f)
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(true)
			.AreaTitle(LOCTEXT("TooltipToolBehaviorExpandableTitle", "Tool Behavior"))
			.BodyContent()
			[
				SAssignNew(CommitOnTextChangeCheckBox, SCheckBox)
				.IsChecked(ECheckBoxState::Checked)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TooltipToolCommitOnChangeLabel", "Commit Tooltip on Text Change"))
				]
			]
		]
	];

	RebuildMemberList();
}

FTooltipTool::EResult STooltipTool::GetUserResponse() const
{
	return UserResponse;
}

FReply STooltipTool::OnButtonClick(FTooltipTool::EResult ButtonID)
{
	ParentWindow->RequestDestroyWindow();
	UserResponse = ButtonID;

	return FReply::Handled();
}

FText STooltipTool::GetSelectedBlueprintText() const
{
	if (BlueprintComboBox->GetSelectedItem().IsValid())
	{
		return FText::FromName(BlueprintComboBox->GetSelectedItem().Get()->AssetName);
	}

	return FText::FromString(TEXT("No Blueprint Selected."));
}

void STooltipTool::UpdateVariableTooltipText(const FText& NewText)
{
	// Early out if text box isn't enabled
	if (!VariableTooltipEditableTextBox->IsEnabled())
	{
		return;
	}

	TArray<TSharedPtr<FBPVariableDescription>> SelectedMembers;
	MemberListView->GetSelectedItems(SelectedMembers);
	if (SelectedMembers.Num() == 1)
	{
		TSharedPtr<FAssetData> BlueprintAsset = BlueprintComboBox->GetSelectedItem();
		UBlueprint* Blueprint = CastChecked<UBlueprint>(BlueprintAsset->GetAsset());
		SelectedMembers[0]->SetMetaData(FBlueprintMetadata::MD_Tooltip, NewText.ToString());
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, SelectedMembers[0]->VarName, nullptr, FBlueprintMetadata::MD_Tooltip, NewText.ToString());
	}
	else
	{
		check(false);
	}
}

void STooltipTool::UpdateCurrentFunctionTooltipText()
{
	TArray<TSharedPtr<FBPFunctionPointers>> SelectedFunctions = FunctionListView->GetSelectedItems();

	if (SelectedFunctions.Num() == 0)
	{
		return;
	}

	check(SelectedFunctions.Num() == 1); //If anything is selected, only one thing should be selected.

	for (int32 i = 0; i < FunctionArgumentDescriptions.Num(); i++)
	{	
		// @TODO: Don't do this
		TSharedRef<const SWidget> Child = FunctionArgumentListView->WidgetFromItem(FunctionArgumentDescriptions[i])->GetContent()->GetChildren()->GetChildAt(1);
		const SEditableTextBox& TooltipBox = static_cast<const SEditableTextBox&>(Child.Get());
		FunctionArgumentDescriptions[i]->Tooltip = TooltipBox.GetText();
	}

	for (int32 i = 0; i < FunctionOutputDescriptions.Num(); i++)
	{
		TSharedRef<const SWidget> Child = FunctionOutputListView->WidgetFromItem(FunctionOutputDescriptions[i])->GetContent()->GetChildren()->GetChildAt(1);
		const SEditableTextBox& TooltipBox = static_cast<const SEditableTextBox&>(Child.Get());
		FunctionOutputDescriptions[i]->Tooltip = TooltipBox.GetText();
	}

	FString RawTooltip = FTooltipStringHelper::ConvertTooltipDataToRawTooltip(FunctionDescriptionTooltipBox->GetText(), FunctionArgumentDescriptions, FunctionOutputDescriptions);
	
	TSharedPtr<FBPFunctionPointers> FunctionPointer = SelectedFunctions[0];
	FunctionPointer->FunctionEntryNode->MetaData.ToolTip = FText::FromString(RawTooltip);
}

void STooltipTool::RebuildMemberList()
{
	Members.Empty();
	FunctionPointers.Empty();
	FunctionArgumentDescriptions.Empty();
	FunctionOutputDescriptions.Empty();

	if (!BlueprintComboBox->GetSelectedItem().IsValid())
	{
		return;
	}

	UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintComboBox->GetSelectedItem().Get()->GetAsset());
	
	// Get variables
	for (FBPVariableDescription Member : Blueprint->NewVariables)
	{
		if ((Member.PropertyFlags & CPF_DisableEditOnInstance) != CPF_DisableEditOnInstance)
		{
			Members.Push(TSharedPtr<FBPVariableDescription>(new FBPVariableDescription(Member)));
		}
	}

	// Get functions
	for (UEdGraph* FunctionGraph : Blueprint->FunctionGraphs)
	{
		if (FunctionGraph->GetFName() != UEdGraphSchema_K2::FN_UserConstructionScript)
		{
			TWeakObjectPtr<UK2Node_EditablePinBase> FuncEntryPtr;
			TWeakObjectPtr<UK2Node_EditablePinBase> FuncResultPtr;
			FBlueprintEditorUtils::GetEntryAndResultNodes(FunctionGraph, FuncEntryPtr, FuncResultPtr);

			UK2Node_FunctionEntry* FunctionEntryNode = Cast<UK2Node_FunctionEntry>(FuncEntryPtr.Get());
			UK2Node_FunctionResult* FunctionResultNode = Cast<UK2Node_FunctionResult>(FuncResultPtr.Get());

			if (FunctionEntryNode != nullptr && FunctionEntryNode->IsEditable() && FunctionEntryNode->GetFunctionFlags() & FUNC_Public)
			{
				FunctionPointers.Push(MakeShared<FBPFunctionPointers>(FunctionEntryNode, FunctionResultNode, FunctionEntryNode->GetGraph()->GetFName()));
			}
		}
	}

	MemberListView->RebuildList();
	FunctionListView->RebuildList();

	if (Members.Num() > 0)
	{
		MemberListView->SetSelection(Members[0], ESelectInfo::Direct);
	}

	if (FunctionPointers.Num() > 0)
	{
		FunctionListView->SetSelection(FunctionPointers[0], ESelectInfo::Direct);
	}
}

#undef LOCTEXT_NAMESPACE
