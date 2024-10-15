#pragma once

#include <vector>
#include <unordered_map>

#include "Engine/Data/AutonoteEnums.h"

#include "GUI/UI/UIBooks.h"

struct GUIWindow_AutonotesBook : public GUIWindow_Book {
    GUIWindow_AutonotesBook();
    virtual ~GUIWindow_AutonotesBook() {}

    virtual void Update() override;

 protected:
    void recalculateCurrentNotesTypePages();

 private:
    int _startingNotesIdx;
    int _currentPage;
    int _currentPageNotes;
    std::vector<int> _activeNotesIdx;
    std::unordered_map<int, int> _notesPerPage;
};

extern AutonoteType autonoteBookDisplayType;
