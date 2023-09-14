#pragma once
#include <vector>
#include <unordered_map>

#include "GUI/UI/UIBooks.h"

struct GUIWindow_QuestBook : public GUIWindow_Book {
    GUIWindow_QuestBook();
    virtual ~GUIWindow_QuestBook() {}

    virtual void Update() override;

 private:
    int _startingQuestIdx;
    int _currentPage;
    int _currentPageQuests;
    std::vector<QuestBit> _activeQuestsIdx;
    std::unordered_map<int, int> _questsPerPage;
};
