#pragma once
#include <vector>
#include <unordered_map>

#include "GUI/UI/UIBooks.h"
#include "Engine/Tables/AutonoteTable.h"

enum class AUTONOTE_TYPE : uint32_t;

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

extern AUTONOTE_TYPE autonoteBookDisplayType;
