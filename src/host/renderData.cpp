/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#include "precomp.h"

#include "renderData.hpp"

#include "dbcs.h"
#include "handle.h"

#include "..\interactivity\inc\ServiceLocator.hpp"

#pragma hdrstop

RenderData::RenderData()
{

}

RenderData::~RenderData()
{

}

const Microsoft::Console::Types::Viewport& RenderData::GetViewport()
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    return gci.GetActiveOutputBuffer().GetViewport();
}

const TextBuffer& RenderData::GetTextBuffer()
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    return gci.GetActiveOutputBuffer().GetTextBuffer();
}

const FontInfo* RenderData::GetFontInfo()
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    return &gci.GetActiveOutputBuffer().GetCurrentFont();
}

const TextAttribute RenderData::GetDefaultBrushColors()
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    return gci.GetActiveOutputBuffer().GetAttributes();
}

const void RenderData::GetColorTable(_Outptr_result_buffer_all_(*pcColors) COLORREF** const ppColorTable, _Out_ size_t* const pcColors)
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    *ppColorTable = const_cast<COLORREF*>(gci.GetColorTable());
    *pcColors = gci.GetColorTableSize();
}

// Method Description:
// - Gets the cursor's position in the buffer, relative to the buffer origin.
// Arguments:
// - <none>
// Return Value:
// - the cursor's position in the buffer relative to the buffer origin.
COORD RenderData::GetCursorPosition() const
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    const auto& cursor = gci.GetActiveOutputBuffer().GetTextBuffer().GetCursor();
    return cursor.GetPosition();
}

// Method Description:
// - Returns wheter the cursor is currently visible or not.
// Arguments:
// - <none>
// Return Value:
// - true if the cursor is currently visible
bool RenderData::IsCursorVisible() const
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    const auto& cursor = gci.GetActiveOutputBuffer().GetTextBuffer().GetCursor();
    return cursor.IsVisible() && cursor.IsOn() && !cursor.IsPopupShown();
}

// Method Description:
// - The height of the cursor, out of 100, where 100 indicates the cursor should
//      be the full height of the cell.
// Arguments:
// - <none>
// Return Value:
// - height of the cursor, out of 100
ULONG RenderData::GetCursorHeight() const
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    const auto& cursor = gci.GetActiveOutputBuffer().GetTextBuffer().GetCursor();
    // Determine cursor height
    ULONG ulHeight = cursor.GetSize();

    // Now adjust the height for the overwrite/insert mode. If we're in overwrite mode, IsDouble will be set.
    // When IsDouble is set, we either need to double the height of the cursor, or if it's already too big,
    // then we need to shrink it by half.
    if (cursor.IsDouble())
    {
        if (ulHeight > 50) // 50 because 50 percent is half of 100 percent which is the max size.
        {
            ulHeight >>= 1;
        }
        else
        {
            ulHeight <<= 1;
        }
    }

    return ulHeight;
}

// Method Description:
// - The CursorType of the cursor. The CursorType is used to determine what
//      shape the cursor should be.
// Arguments:
// - <none>
// Return Value:
// - the CursorType of the cursor.
CursorType RenderData::GetCursorStyle() const
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    const auto& cursor = gci.GetActiveOutputBuffer().GetTextBuffer().GetCursor();
    return cursor.GetType();
}

// Method Description:
// - Get the color of the cursor. If the color is INVALID_COLOR, the cursor
//      should be drawn by inverting the color of the cursor.
// Arguments:
// - <none>
// Return Value:
// - the color of the cursor.
COLORREF RenderData::GetCursorColor() const
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    const auto& cursor = gci.GetActiveOutputBuffer().GetTextBuffer().GetCursor();
    return cursor.GetColor();
}

// Method Description:
// - Returns true if the cursor should be drawn twice as wide as usual because
//      the cursor is currently over a cell with a double-wide character in it.
// Arguments:
// - <none>
// Return Value:
// - true if the cursor should be drawn twice as wide as usual
bool RenderData::IsCursorDoubleWidth() const
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    return gci.GetActiveOutputBuffer().CursorIsDoubleWidth();
}

const ConsoleImeInfo* RenderData::GetImeData()
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    return &gci.ConsoleIme;
}

const TextBuffer& RenderData::GetImeCompositionStringBuffer(_In_ size_t iIndex)
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    THROW_HR_IF(E_INVALIDARG, iIndex >= gci.ConsoleIme.ConvAreaCompStr.size());
    return gci.ConsoleIme.ConvAreaCompStr[iIndex].GetTextBuffer();
}

std::vector<SMALL_RECT> RenderData::GetSelectionRects()
{
    return Selection::Instance().GetSelectionRects();
}

const bool RenderData::IsGridLineDrawingAllowed()
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    // If virtual terminal output is set, grid line drawing is a must. It is always allowed.
    if (WI_IsFlagSet(gci.GetActiveOutputBuffer().OutputMode, ENABLE_VIRTUAL_TERMINAL_PROCESSING))
    {
        return true;
    }
    else
    {
        // If someone explicitly asked for worldwide line drawing, enable it.
        if (gci.IsGridRenderingAllowedWorldwide())
        {
            return true;
        }
        else
        {
            // Otherwise, for compatibility reasons with legacy applications that used the additional CHAR_INFO bits by accident or for their own purposes,
            // we must enable grid line drawing only in a DBCS output codepage. (Line drawing historically only worked in DBCS codepages.)
            // The only known instance of this is Image for Windows by TeraByte, Inc. (TeryByte Unlimited) which used the bits accidentally and for no purpose
            //   (according to the app developer) in conjunction with the Borland Turbo C cgscrn library.
            return !!IsAvailableEastAsianCodePage(gci.OutputCP);
        }
    }
}

const std::wstring RenderData::GetConsoleTitle() const
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    return gci.GetTitleAndPrefix();
}

const COLORREF RenderData::GetForegroundColor(const TextAttribute& attr) const
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    return gci.LookupForegroundColor(attr);
}

const COLORREF RenderData::GetBackgroundColor(const TextAttribute& attr) const
{
    const CONSOLE_INFORMATION& gci = ServiceLocator::LocateGlobals().getConsoleInformation();
    return gci.LookupBackgroundColor(attr);
}

void RenderData::LockConsole()
{
    ::LockConsole();
}

void RenderData::UnlockConsole()
{
    ::UnlockConsole();
}
