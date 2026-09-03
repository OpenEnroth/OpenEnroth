#include <concepts>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Tsv/TsvReader.h"

#include "Utility/Memory/Blob.h"

// A `TsvLine` is a range of its cells, and a view, even though it isn't default-constructible - a default-constructed
// line would point at no reader and have no meaning. C++20 originally required views to be default-initializable,
// P2325R3 dropped that.
static_assert(std::ranges::range<TsvLine>);
static_assert(std::ranges::view<TsvLine>);
static_assert(std::ranges::forward_range<TsvLine>);
static_assert(!std::default_initializable<TsvLine>);

// `f(std::string_view)` + `f(std::string &&)` is a sane overload set, and `Split.h` is exactly that shape, with
// the string overload deleted. A cell has to pick the view - back when it converted to both, the call was
// ambiguous, because two user-defined conversions rank the same.
static std::string_view pickedOverload(std::string_view) {
    return "string_view";
}

static std::string_view pickedOverload(std::string &&) {
    return "string";
}

UNIT_TEST(TsvCell, PicksTheStringViewOverload) {
    for (TsvLine cells : TsvReader("hello\tworld\r\n")) {
        EXPECT_EQ(pickedOverload(cells[0]), "string_view");

        // And both of these still work.
        std::string string = cells[0];
        std::string_view view = cells[1];
        EXPECT_EQ(string, "hello");
        EXPECT_EQ(view, "world");
    }
}

UNIT_TEST(TsvReader, Basics) {
    TsvReader reader("a\tb\tc\r\nd\te\tf\r\n");

    std::vector<std::string> values;
    for (TsvLine cells : reader) {
        EXPECT_EQ(cells.size(), 3);
        for (size_t i = 0; i < cells.size(); i++)
            values.push_back(std::string(cells[i]));
    }

    EXPECT_EQ(values, (std::vector<std::string>{"a", "b", "c", "d", "e", "f"}));
}

UNIT_TEST(TsvReader, HeaderLines) {
    std::vector<std::string> lines;
    for (TsvLine cells : TsvReader("h1\r\nh2\r\ndata\r\n").drop(2))
        lines.push_back(std::string(cells.str()));

    EXPECT_EQ(lines, (std::vector<std::string>{"data"}));
}

UNIT_TEST(TsvReader, DropCountsEmptyRows) {
    // Header blocks do contain all-tabs rows, so dropping the header has to happen before the empty rows are
    // filtered out - otherwise the drop eats the first data rows.
    std::vector<std::string> lines;
    for (TsvLine cells : TsvReader("\t\t\r\nhead\r\n\t\t\r\ndata\r\n").drop(3).skip(&TsvLine::isBlank))
        lines.push_back(std::string(cells.str()));

    EXPECT_EQ(lines, (std::vector<std::string>{"data"}));
}

UNIT_TEST(TsvReader, EmptyRowsAreReadUnlessSkipped) {
    std::string_view data = "a\r\n\r\n\t\t\t\r\n   \t \r\nb\r\n";

    std::vector<std::string> all;
    for (TsvLine cells : TsvReader(data))
        all.push_back(std::string(cells[0]));
    EXPECT_EQ(all, (std::vector<std::string>{"a", "", "", "", "b"}));

    // Excel pads a table with runs of tabs past the last data row.
    std::vector<std::string> nonEmpty;
    for (TsvLine cells : TsvReader(data).skip(&TsvLine::isBlank))
        nonEmpty.push_back(std::string(cells[0]));
    EXPECT_EQ(nonEmpty, (std::vector<std::string>{"a", "b"}));
}

UNIT_TEST(TsvReader, LineWithEmptyFirstCellIsNotEmpty) {
    // Data rows whose first cell is empty are real, and must not be mistaken for padding.
    TsvReader reader("\tA\t\t0\r\n");

    size_t count = 0;
    for (TsvLine cells : reader) {
        count++;
        EXPECT_TRUE(cells[0].empty());
        EXPECT_EQ(cells[1], "A");
        EXPECT_EQ(cells[3], "0");
    }
    EXPECT_EQ(count, 1);
}

UNIT_TEST(TsvReader, ReadingPastTheLastCell) {
    TsvReader reader("a\tb\r\n");

    for (TsvLine cells : reader) {
        EXPECT_EQ(cells.size(), 2);
        EXPECT_TRUE(cells[2].empty()); // Ragged tables need no resize() at the call site.
        EXPECT_TRUE(cells[100].empty());
        EXPECT_EQ(cells[100].empty() ? 7 : cells[100].as<int>(), 7); // How callers spell a default.
    }
}

UNIT_TEST(TsvReader, Trimming) {
    // Cells routinely carry leading spaces, e.g. " 1-5".
    TsvReader reader(" a \t  \t 42 \r\n");

    for (TsvLine cells : reader) {
        EXPECT_EQ(cells[0], "a");
        EXPECT_TRUE(cells[1].empty());
        EXPECT_EQ(cells[2].as<int>(), 42);
    }
}

UNIT_TEST(TsvReader, Unquoting) {
    TsvReader reader("\"quoted\"\tplain\t\"\"\r\n");

    for (TsvLine cells : reader) {
        EXPECT_EQ(cells[0], "quoted");
        EXPECT_EQ(cells[1], "plain");
        EXPECT_TRUE(cells[2].empty());
    }
}

UNIT_TEST(TsvReader, UnquotingDoubledQuotes) {
    // This is what Excel writes out for a cell containing a double quote.
    TsvReader reader("\"small laboratory.  Good place for \"\"experimental\"\" items.\"\r\n");

    for (TsvLine cells : reader)
        EXPECT_EQ(cells[0], "small laboratory.  Good place for \"experimental\" items.");
}

UNIT_TEST(TsvReader, UnquotedCellsKeepDoubledQuotes) {
    // The `""` escaping only applies inside a quoted cell.
    TsvReader reader("a\"\"b\r\n");

    for (TsvLine cells : reader)
        EXPECT_EQ(cells[0], "a\"\"b");
}

UNIT_TEST(TsvReader, UnquotingUnterminatedQuote) {
    TsvReader reader("\"no closing quote\r\n");

    for (TsvLine cells : reader)
        EXPECT_EQ(cells[0], "no closing quote");
}

UNIT_TEST(TsvReader, TwoUnquotedCellsCoexist) {
    // A single shared scratch buffer would make the second cell overwrite the first one.
    TsvReader reader("\"a\"\"b\"\t\"c\"\"d\"\r\n");

    for (TsvLine cells : reader) {
        std::string_view first = cells[0];
        std::string_view second = cells[1];
        EXPECT_EQ(first, "a\"b");
        EXPECT_EQ(second, "c\"d");
    }
}

UNIT_TEST(TsvReader, ScratchIsReusedBetweenLines) {
    TsvReader reader("\"a\"\"a\"\r\n\"b\"\"b\"\r\n");

    std::vector<std::string> values;
    for (TsvLine cells : reader)
        values.push_back(std::string(cells[0]));

    EXPECT_EQ(values, (std::vector<std::string>{"a\"a", "b\"b"}));
}

UNIT_TEST(TsvReader, LineNumbersAreOneBasedAndCountEverything) {
    std::vector<size_t> numbers;
    for (TsvLine cells : TsvReader("h\r\n\r\na\r\nb\r\n").drop(1).skip(&TsvLine::isBlank))
        numbers.push_back(cells.lineNumber());

    EXPECT_EQ(numbers, (std::vector<size_t>{3, 4}));
}

UNIT_TEST(TsvReader, ParseErrorMentionsPosition) {
    TsvReader reader("1\tnope\r\n", "table.txt");

    for (TsvLine cells : reader) {
        EXPECT_EQ(cells[0].as<int>(), 1);
        EXPECT_THROW_MESSAGE((void) cells[1].as<int>(), "table.txt:1:2"); // 1-based line and column.
    }
}

UNIT_TEST(TsvCell, SubstrKeepsThePosition) {
    // Taking a substring used to slice the cell back to a plain `std::string_view`, and the parse error that came
    // out of it named no file and no column.
    TsvReader reader("skip\tE12\tEnope\r\n", "table.txt");

    for (TsvLine cells : reader) {
        EXPECT_EQ(cells[1].substr(1).as<int>(), 12);
        EXPECT_THROW_MESSAGE((void) cells[2].substr(1).as<int>(), "table.txt:1:3"); // 1-based line and column.
    }
}

UNIT_TEST(TsvLine, IsARangeOfCells) {
    // HostilityTable and the npcdist parser walk all cells instead of indexing them.
    TsvReader reader("skip\t10\t20\t30\r\n");

    for (TsvLine cells : reader) {
        std::vector<std::string> tail;
        for (std::string_view cell : cells.drop(1))
            tail.push_back(std::string(cell));
        EXPECT_EQ(tail, (std::vector<std::string>{"10", "20", "30"}));
    }
}

UNIT_TEST(TsvLine, CellsReportPosition) {
    // Iterating a line used to yield plain `std::string_view`s, so the tables that walk all cells had to reach for
    // `fromString` and lost the position of the offending cell in the file.
    TsvReader reader("skip\t10\tnope\r\n", "table.txt");

    for (TsvLine cells : reader)
        for (TsvCell cell : cells.drop(2))
            EXPECT_THROW_MESSAGE((void) cell.as<int>(), "table.txt:1:3"); // 1-based line and column.
}

UNIT_TEST(TsvReader, EmptyInput) {
    TsvReader reader("");

    size_t count = 0;
    for ([[maybe_unused]] TsvLine cells : reader)
        count++;

    EXPECT_EQ(count, 0);
}

UNIT_TEST(TsvReader, HeaderLongerThanInput) {
    size_t count = 0;
    for ([[maybe_unused]] TsvLine cells : TsvReader("a\r\n").drop(5))
        count++;

    EXPECT_EQ(count, 0);
}

UNIT_TEST(TsvReader, NoTrailingNewline) {
    TsvReader reader("a\tb");

    for (TsvLine cells : reader) {
        EXPECT_EQ(cells.size(), 2);
        EXPECT_EQ(cells[1], "b");
    }
}

UNIT_TEST(TsvReader, BareNewlinesStayInsideCells) {
    // Paragraph breaks inside quoted values are bare LFs, so only CRLF separates records.
    TsvReader reader("1\t\"first\n\nsecond\"\r\n2\tplain\r\n");

    std::vector<std::string> values;
    for (TsvLine cells : reader)
        values.push_back(std::string(cells[1]));

    EXPECT_EQ(values, (std::vector<std::string>{"first\n\nsecond", "plain"}));
}

UNIT_TEST(TsvReader, ScratchGrowthKeepsEarlierCellsValid) {
    // Growing the scratch buffer must not move the strings that the already-parsed cells of the line point into.
    // That is why it is a deque, a vector would relocate its elements and dangle every earlier cell.
    std::string data;
    for (int i = 0; i < 100; i++)
        data += "\"a\"\"" + std::to_string(i) + "\"\t";
    data += "last\r\n";

    TsvReader reader(data);
    for (TsvLine cells : reader) {
        EXPECT_EQ(cells.size(), 101);
        EXPECT_EQ(cells[0], "a\"0");
        EXPECT_EQ(cells[63], "a\"63");
        EXPECT_EQ(cells[99], "a\"99");
        EXPECT_EQ(cells[100], "last");
    }
}

UNIT_TEST(TsvReader, BlobConstructor) {
    Blob blob = Blob::view("a\tb\r\n").withDisplayPath("test.txt");

    for (TsvLine cells : TsvReader(blob)) {
        EXPECT_EQ(cells[1], "b");
        EXPECT_THROW_MESSAGE((void) cells[1].as<int>(), "test.txt:1:2");
    }
}
