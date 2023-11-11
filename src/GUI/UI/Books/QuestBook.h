#pragma once
#include <vector>
#include <unordered_map>

#include "GUI/UI/UIBooks.h"

struct GUIWindow_QuestBook : public GUIWindow_Book {
    GUIWindow_QuestBook();
    virtual ~GUIWindow_QuestBook() {}

    virtual void Update() override;

 private:
    int _startingQuestIdx = 0;
    int _currentPage = 0;
    int _currentPageQuests = 0;
    std::vector<QuestBit> _activeQuestsIdx;
    std::unordered_map<int, int> _questsPerPage;
};
