// wintourguide.cpp
//
// Copyright (C) 2001, Chris Laurel <claurel@shatters.net>
//
// Space 'tour guide' dialog for Windows.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#include "wintourguide.h"

#include <string>
#include <sstream>
#include <algorithm>
#include <set>

#include <celutil/winutil.h>

#include "res/resource.h"

#include <commctrl.h>


using namespace Eigen;
using namespace std;

namespace util = celestia::util;


BOOL APIENTRY TourGuideProc(HWND hDlg,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam)
{
    TourGuide* tourGuide = reinterpret_cast<TourGuide*>(GetWindowLongPtr(hDlg, DWLP_USER));

    switch (message)
    {
    case WM_INITDIALOG:
        {
            TourGuide* guide = reinterpret_cast<TourGuide*>(lParam);
            if (guide == NULL)
                return EndDialog(hDlg, 0);
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);

//          guide->selectedDest = NULL;

            HWND hwnd = GetDlgItem(hDlg, IDC_COMBO_TOURGUIDE);
            const DestinationList* destinations = guide->appCore->getDestinations();
            if (hwnd != NULL && destinations != NULL)
            {
                Destination* dest = (*destinations)[0];
                guide->selectedDest = dest;
                for (DestinationList::const_iterator iter = destinations->begin();
                     iter != destinations->end(); iter++)
                {
                    Destination* dest = *iter;
                    if (dest != NULL)
                    {
                        SendMessage(hwnd, CB_INSERTSTRING, -1,
                                    reinterpret_cast<LPARAM>(util::UTF8ToCurrentCP(dest->name).c_str()));
                    }
                }

                if (destinations->size() > 0)
                {
                    SendMessage(hwnd, CB_SETCURSEL, 0, 0);
                    SetDlgItemText(hDlg,
                                   IDC_TEXT_DESCRIPTION,
                                   util::UTF8ToCurrentCP((*destinations)[0]->description).c_str());
                }
            }
        }
        return(TRUE);

    case WM_DESTROY:
        if (tourGuide != NULL && tourGuide->parent != NULL)
        {
            SendMessage(tourGuide->parent, WM_COMMAND, IDCLOSE,
                        reinterpret_cast<LPARAM>(tourGuide));
        }
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            if (tourGuide != NULL && tourGuide->parent != NULL)
            {
                SendMessage(tourGuide->parent, WM_COMMAND, IDCLOSE,
                            reinterpret_cast<LPARAM>(tourGuide));
            }
            EndDialog(hDlg, 0);
            return TRUE;
        }
        else if (LOWORD(wParam) == IDC_BUTTON_GOTO)
        {
            Simulation* sim = tourGuide->appCore->getSimulation();
            if (tourGuide->selectedDest != NULL && sim != NULL)
            {
                Selection sel = sim->findObjectFromPath(tourGuide->selectedDest->target);
                if (!sel.empty())
                {
                    sim->follow();
                    sim->setSelection(sel);
                    if (tourGuide->selectedDest->distance <= 0)
                    {
                        // Use the default distance
                        sim->gotoSelection(5.0,
                                           Vector3f::UnitY(),
                                           ObserverFrame::ObserverLocal);
                    }
                    else
                    {
                        sim->gotoSelection(5.0,
                                           tourGuide->selectedDest->distance,
                                           Vector3f::UnitY(),
                                           ObserverFrame::ObserverLocal);
                    }
                }
            }
        }
        else if (LOWORD(wParam) == IDC_COMBO_TOURGUIDE)
        {
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                HWND hwnd = reinterpret_cast<HWND>(lParam);
                int item = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
                const DestinationList* destinations = tourGuide->appCore->getDestinations();
                if (item != CB_ERR && item < (int) destinations->size())
                {
                    Destination* dest = (*destinations)[item];
                    SetDlgItemText(hDlg,
                                   IDC_TEXT_DESCRIPTION,
                                   util::UTF8ToCurrentCP(dest->description).c_str());
                    tourGuide->selectedDest = dest;
                }
            }
        }
        break;
    }

    return FALSE;
}


TourGuide::TourGuide(HINSTANCE appInstance,
                     HWND _parent,
                     CelestiaCore* _appCore) :
    appCore(_appCore),
    selectedDest(NULL),
    parent(_parent)
{
    hwnd = CreateDialogParam(appInstance,
                             MAKEINTRESOURCE(IDD_TOURGUIDE),
                             parent,
                             (DLGPROC)TourGuideProc,
                             reinterpret_cast<LONG_PTR>(this));
}
