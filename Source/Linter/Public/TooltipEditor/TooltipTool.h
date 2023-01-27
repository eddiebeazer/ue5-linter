// Copyright 2016 Gamemakin LLC. All Rights Reserved.

#pragma once

#include "EditorStyleSet.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/SUserWidget.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Views/SListView.h"
#include "AssetData.h"
#include "Engine/Blueprint.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"

#include "TooltipStringHelper.h"

#define LOCTEXT_NAMESPACE "LinterTooltipTool"


/**
* Helper struct for showing function tooltip widgets
**/
struct FBPFunctionPointers
{
	UK2Node_FunctionEntry* FunctionEntryNode;
	UK2Node_FunctionResult* FunctionResultNode;
	FName FunctionName;

	FBPFunctionPointers()
	{
	}

	FBPFunctionPointers(UK2Node_FunctionEntry* InFunctionEntryNode, UK2Node_FunctionResult* InFunctionResultNode, FName InFunctionName)
		: FunctionEntryNode(InFunctionEntryNode)
		, FunctionResultNode(InFunctionResultNode)
		, FunctionName(InFunctionName)
	{
	}
};

/**
* FTooltipTool
*
* Wrapper class for STooltipTool. This class creates and launches a dialog then awaits the
* result to return to the user.
*/
class FTooltipTool
{
public:
	enum EResult
	{
		Cancel = 0,			// No/Cancel, normal usage would stop the current action
		Confirm = 1,		// Yes/Ok/Etc, normal usage would continue with action
	};

	FTooltipTool(const TArray<FAssetData> Assets);

	/**  Shows the dialog box and waits for the user to respond. */
	EResult ShowModal();

	TArray<FAssetData> BlueprintsInternal;
	TArray<TSharedPtr<FAssetData>> Blueprints;

private:

	/** Cached pointer to the modal window */
	TSharedPtr<SWindow> DialogWindow;

	/** Cached pointer to the batch rename tool widget */
	TSharedPtr<class STooltipTool> DialogWidget;
};

/**
* Slate panel for batch renaming
*/
class STooltipTool : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(STooltipTool)
	{}
	/** Window in which this widget resides */
	SLATE_ATTRIBUTE(TSharedPtr<SWindow>, ParentWindow)
	SLATE_ATTRIBUTE(TArray<TSharedPtr<FAssetData>>, Blueprints)
		SLATE_END_ARGS()

		/**
		* Constructs this widget
		*
		* @param	InArgs	The declaration data for this widget
		*/
		void Construct(const FArguments& InArgs);

	/**
	* Returns the EResult of the button which the user pressed. Closing of the dialog
	* in any other way than clicking "Ok" results in this returning a "Cancel" value
	*/
	FTooltipTool::EResult GetUserResponse() const;

private:

	/**
	* Handles when a button is pressed, should be bound with appropriate EResult Key
	*
	* @param ButtonID - The return type of the button which has been pressed.
	*/
	FReply OnButtonClick(FTooltipTool::EResult ButtonID);

	FText GetSelectedBlueprintText() const;

	void UpdateVariableTooltipText(const FText& NewText);
	void UpdateCurrentFunctionTooltipText();

	void RebuildMemberList();

	/** Stores the users response to this dialog */
	FTooltipTool::EResult UserResponse;

	/** Pointer to the window which holds this Widget, required for modal control */
	TSharedPtr<SWindow>	ParentWindow;

public:

	
	
	TAttribute<TArray<TSharedPtr<FAssetData>>> Blueprints;
	TSharedPtr<SComboBox<TSharedPtr<FAssetData>>> BlueprintComboBox;

	FText CurrentFunctionDescription;

	TArray<TSharedPtr<FBPFunctionPointers>> FunctionPointers;
	TSharedPtr<SListView<TSharedPtr<FBPFunctionPointers>>> FunctionListView;
	TSharedPtr<SMultiLineEditableTextBox> FunctionDescriptionTooltipBox;
	TArray<TSharedPtr<FBPFunctionArgumentDescription>> FunctionArgumentDescriptions;
	TSharedPtr<SListView<TSharedPtr<FBPFunctionArgumentDescription>>> FunctionArgumentListView;
	TArray<TSharedPtr<FBPFunctionArgumentDescription>> FunctionOutputDescriptions;
	TSharedPtr<SListView<TSharedPtr<FBPFunctionArgumentDescription>>> FunctionOutputListView;

	
	TArray<TSharedPtr<FBPVariableDescription>> Members;
	TSharedPtr<SListView<TSharedPtr<FBPVariableDescription>>> MemberListView;
	TSharedPtr<SMultiLineEditableTextBox> VariableTooltipEditableTextBox;

	TSharedPtr<SButton> CommitTextButton;
	TSharedPtr<SCheckBox> CommitOnTextChangeCheckBox;
};

#undef LOCTEXT_NAMESPACE