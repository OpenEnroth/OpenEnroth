#include <string>
#include <string_view>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Library/Tsv/TsvReader.h"

#include "Utility/Memory/Blob.h"

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
    for (TsvLine line : TsvReader("hello\tworld\r\n")) {
        EXPECT_EQ(pickedOverload(line[0]), "string_view");

        // And both of these still work.
        std::string string = line[0];
        std::string_view view = line[1];
        EXPECT_EQ(string, "hello");
        EXPECT_EQ(view, "world");
    }
}

UNIT_TEST(TsvReader, Basics) {
    TsvReader reader("a\tb\tc\r\nd\te\tf\r\n");

    std::vector<std::string> cells;
    for (TsvLine line : reader) {
        EXPECT_EQ(line.size(), 3);
        for (size_t i = 0; i < line.size(); i++)
            cells.push_back(std::string(line[i]));
    }

    EXPECT_EQ(cells, (std::vector<std::string>{"a", "b", "c", "d", "e", "f"}));
}

UNIT_TEST(TsvReader, HeaderLines) {
    std::vector<std::string> lines;
    for (TsvLine line : TsvReader("h1\r\nh2\r\ndata\r\n").drop(2))
        lines.push_back(std::string(line.str()));

    EXPECT_EQ(lines, (std::vector<std::string>{"data"}));
}

UNIT_TEST(TsvReader, DropCountsEmptyRows) {
    // Header blocks do contain all-tabs rows, so dropping the header has to happen before the empty rows are
    // filtered out - otherwise the drop eats the first data rows.
    std::vector<std::string> lines;
    for (TsvLine line : TsvReader("\t\t\r\nhead\r\n\t\t\r\ndata\r\n").drop(3).skip(&TsvLine::isEmpty))
        lines.push_back(std::string(line.str()));

    EXPECT_EQ(lines, (std::vector<std::string>{"data"}));
}

UNIT_TEST(TsvReader, EmptyRowsAreReadUnlessSkipped) {
    std::string_view data = "a\r\n\r\n\t\t\t\r\n   \t \r\nb\r\n";

    std::vector<std::string> all;
    for (TsvLine line : TsvReader(data))
        all.push_back(std::string(line[0]));
    EXPECT_EQ(all, (std::vector<std::string>{"a", "", "", "", "b"}));

    // Excel pads a table with runs of tabs past the last data row.
    std::vector<std::string> nonEmpty;
    for (TsvLine line : TsvReader(data).skip(&TsvLine::isEmpty))
        nonEmpty.push_back(std::string(line[0]));
    EXPECT_EQ(nonEmpty, (std::vector<std::string>{"a", "b"}));
}

UNIT_TEST(TsvReader, LineWithEmptyFirstCellIsNotEmpty) {
    // Data rows whose first cell is empty are real, and must not be mistaken for padding.
    TsvReader reader("\tA\t\t0\r\n");

    size_t count = 0;
    for (TsvLine line : reader) {
        count++;
        EXPECT_TRUE(line[0].empty());
        EXPECT_EQ(line[1], "A");
        EXPECT_EQ(line[3], "0");
    }
    EXPECT_EQ(count, 1);
}

UNIT_TEST(TsvReader, ReadingPastTheLastCell) {
    TsvReader reader("a\tb\r\n");

    for (TsvLine line : reader) {
        EXPECT_EQ(line.size(), 2);
        EXPECT_TRUE(line[2].empty()); // Ragged tables need no resize() at the call site.
        EXPECT_TRUE(line[100].empty());
        EXPECT_EQ(line[100].empty() ? 7 : line[100].as<int>(), 7); // How callers spell a default.
    }
}

UNIT_TEST(TsvReader, Trimming) {
    // Cells routinely carry leading spaces, e.g. " 1-5".
    TsvReader reader(" a \t  \t 42 \r\n");

    for (TsvLine line : reader) {
        EXPECT_EQ(line[0], "a");
        EXPECT_TRUE(line[1].empty());
        EXPECT_EQ(line[2].as<int>(), 42);
    }
}

UNIT_TEST(TsvReader, Unquoting) {
    TsvReader reader("\"quoted\"\tplain\t\"\"\r\n");

    for (TsvLine line : reader) {
        EXPECT_EQ(line[0], "quoted");
        EXPECT_EQ(line[1], "plain");
        EXPECT_TRUE(line[2].empty());
    }
}

UNIT_TEST(TsvReader, UnquotingDoubledQuotes) {
    // This is what Excel writes out for a cell containing a double quote.
    TsvReader reader("\"small laboratory.  Good place for \"\"experimental\"\" items.\"\r\n");

    for (TsvLine line : reader)
        EXPECT_EQ(line[0], "small laboratory.  Good place for \"experimental\" items.");
}

UNIT_TEST(TsvReader, UnquotedCellsKeepDoubledQuotes) {
    // The `""` escaping only applies inside a quoted cell.
    TsvReader reader("a\"\"b\r\n");

    for (TsvLine line : reader)
        EXPECT_EQ(line[0], "a\"\"b");
}

UNIT_TEST(TsvReader, UnquotingUnterminatedQuote) {
    TsvReader reader("\"no closing quote\r\n");

    for (TsvLine line : reader)
        EXPECT_EQ(line[0], "no closing quote");
}

UNIT_TEST(TsvReader, TwoUnquotedCellsCoexist) {
    // A single shared scratch buffer would make the second cell overwrite the first one.
    TsvReader reader("\"a\"\"b\"\t\"c\"\"d\"\r\n");

    for (TsvLine line : reader) {
        std::string_view first = line[0];
        std::string_view second = line[1];
        EXPECT_EQ(first, "a\"b");
        EXPECT_EQ(second, "c\"d");
    }
}

UNIT_TEST(TsvReader, ScratchIsReusedBetweenLines) {
    TsvReader reader("\"a\"\"a\"\r\n\"b\"\"b\"\r\n");

    std::vector<std::string> cells;
    for (TsvLine line : reader)
        cells.push_back(std::string(line[0]));

    EXPECT_EQ(cells, (std::vector<std::string>{"a\"a", "b\"b"}));
}

UNIT_TEST(TsvReader, LineNumbersAreOneBasedAndCountEverything) {
    std::vector<size_t> numbers;
    for (TsvLine line : TsvReader("h\r\n\r\na\r\nb\r\n").drop(1).skip(&TsvLine::isEmpty))
        numbers.push_back(line.lineNumber());

    EXPECT_EQ(numbers, (std::vector<size_t>{3, 4}));
}

UNIT_TEST(TsvReader, ParseErrorMentionsPosition) {
    TsvReader reader("1\tnope\r\n", "table.txt");

    for (TsvLine line : reader) {
        EXPECT_EQ(line[0].as<int>(), 1);
        EXPECT_THROW_MESSAGE((void) line[1].as<int>(), "table.txt:1:2"); // 1-based line and column.
    }
}

UNIT_TEST(TsvReader, Cells) {
    // HostilityTable and the monster placement table walk all cells instead of indexing them.
    TsvReader reader("skip\t10\t20\t30\r\n");

    for (TsvLine line : reader) {
        std::vector<std::string> tail;
        for (std::string_view cell : line.cells().drop(1))
            tail.push_back(std::string(cell));
        EXPECT_EQ(tail, (std::vector<std::string>{"10", "20", "30"}));
    }
}

UNIT_TEST(TsvReader, EmptyInput) {
    TsvReader reader("");

    size_t count = 0;
    for ([[maybe_unused]] TsvLine line : reader)
        count++;

    EXPECT_EQ(count, 0);
}

UNIT_TEST(TsvReader, HeaderLongerThanInput) {
    size_t count = 0;
    for ([[maybe_unused]] TsvLine line : TsvReader("a\r\n").drop(5))
        count++;

    EXPECT_EQ(count, 0);
}

UNIT_TEST(TsvReader, NoTrailingNewline) {
    TsvReader reader("a\tb");

    for (TsvLine line : reader) {
        EXPECT_EQ(line.size(), 2);
        EXPECT_EQ(line[1], "b");
    }
}

UNIT_TEST(TsvReader, BareNewlinesStayInsideCells) {
    // Paragraph breaks inside quoted cells are bare LFs, so only CRLF separates records.
    TsvReader reader("1\t\"first\n\nsecond\"\r\n2\tplain\r\n");

    std::vector<std::string> cells;
    for (TsvLine line : reader)
        cells.push_back(std::string(line[1]));

    EXPECT_EQ(cells, (std::vector<std::string>{"first\n\nsecond", "plain"}));
}

UNIT_TEST(TsvReader, MoreCellsThanInlineCapacity) {
    // The scratch buffer has 64 inline slots. Growing it while parsing a line used to move the strings that the
    // already-parsed cells of that line were pointing into.
    std::string line;
    for (int i = 0; i < 100; i++)
        line += "\"a\"\"" + std::to_string(i) + "\"\t";
    line += "last\r\n";

    TsvReader reader(line);
    for (TsvLine parsed : reader) {
        EXPECT_EQ(parsed.size(), 101);
        EXPECT_EQ(parsed[0], "a\"0");
        EXPECT_EQ(parsed[63], "a\"63");
        EXPECT_EQ(parsed[99], "a\"99");
        EXPECT_EQ(parsed[100], "last");
    }
}

UNIT_TEST(TsvReader, BlobConstructor) {
    Blob blob = Blob::view("a\tb\r\n").withDisplayPath("test.txt");

    for (TsvLine line : TsvReader(blob)) {
        EXPECT_EQ(line[1], "b");
        EXPECT_THROW_MESSAGE((void) line[1].as<int>(), "test.txt:1:2");
    }
}
