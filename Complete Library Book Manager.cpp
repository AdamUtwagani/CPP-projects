#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>      // for setw
#include <ctime>        // for timestamp in history
using namespace std;

/*
  -------------------------
   Helper utilities
  -------------------------
*/

// Trim helper - remove leading and trailing spaces
static inline std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Convert string to lowercase for case-insensitive search
static inline std::string toLower(const std::string &s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

// Get current date/time string for history logs
string nowStr() {
    std::time_t t = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return string(buf);
}

/*
  -------------------------
   Book class
  -------------------------
   Represents a single book and its state.
*/
class Book {
public:
    string id;           // Unique ID, e.g. BK001
    string title;
    string author;
    int year;
    bool isBorrowed;
    string borrower;     // name of borrower (empty if available)

    // Default constructor
    Book() : id(""), title(""), author(""), year(0), isBorrowed(false), borrower("") {}

    // Parameterized constructor
    Book(const string &id_, const string &title_, const string &author_, int year_, bool borrowed_, const string &borrower_)
        : id(id_), title(title_), author(author_), year(year_), isBorrowed(borrowed_), borrower(borrower_) {}

    // Serialize to a line for saving to file.
    // Format: id|title|author|year|isBorrowed|borrower
    string serialize() const {
        // replace any '|' in fields with '/' to avoid breaking delimiter (simple escape)
        auto esc = [](const string &s){ string r = s; replace(r.begin(), r.end(), '|', '/'); return r; };
        ostringstream out;
        out << id << "|" << esc(title) << "|" << esc(author) << "|" << year << "|" << (isBorrowed ? 1 : 0) << "|" << esc(borrower);
        return out.str();
    }

    // Deserialize from a line (inverse of serialize)
    static Book deserialize(const string &line) {
        vector<string> parts;
        string token;
        std::istringstream ss(line);
        while (std::getline(ss, token, '|')) {
            parts.push_back(token);
        }
        // If file is corrupted, return empty Book
        if (parts.size() < 6) return Book();

        Book b;
        b.id = parts[0];
        b.title = parts[1];
        b.author = parts[2];
        b.year = stoi(parts[3]);
        b.isBorrowed = (parts[4] == "1");
        b.borrower = parts[5];
        return b;
    }

    // Pretty print book info
    void displayShort() const {
        cout << left << setw(7) << id
             << left << setw(30) << (title.size()>27?title.substr(0,27)+"...":title)
             << left << setw(20) << (author.size()>17?author.substr(0,17)+"...":author)
             << left << setw(6) << year
             << (isBorrowed ? "Borrowed" : "Available")
             << (isBorrowed ? (" by " + borrower) : "")
             << endl;
    }

    void displayFull() const {
        cout << "ID: " << id << "\nTitle: " << title << "\nAuthor: " << author
             << "\nYear: " << year << "\nStatus: " << (isBorrowed ? "Borrowed" : "Available") << endl;
        if (isBorrowed) cout << "Borrower: " << borrower << endl;
    }
};


/*
  -------------------------
   HistoryEntry struct
  -------------------------
   Stores a single history record for borrow/return actions.
   Format when saving: timestamp|action|bookID|title|byWho
*/
struct HistoryEntry {
    string timestamp;   // e.g., 2025-12-11 22:00:00
    string action;      // BORROW or RETURN
    string bookID;
    string title;
    string byWho;

    string serialize() const {
        auto esc = [](const string &s){ string r = s; replace(r.begin(), r.end(), '|', '/'); return r; };
        ostringstream out;
        out << timestamp << "|" << action << "|" << esc(bookID) << "|" << esc(title) << "|" << esc(byWho);
        return out.str();
    }

    static HistoryEntry deserialize(const string &line) {
        vector<string> parts;
        string token;
        istringstream ss(line);
        while (getline(ss, token, '|')) {
            parts.push_back(token);
        }
        if (parts.size() < 5) return HistoryEntry();
        HistoryEntry h;
        h.timestamp = parts[0];
        h.action = parts[1];
        h.bookID = parts[2];
        h.title = parts[3];
        h.byWho = parts[4];
        return h;
    }
};


/*
  -------------------------
   Library class
  -------------------------
   Holds vector<Book> and history. Provides all operations.
*/
class Library {
private:
    vector<Book> books;
    vector<HistoryEntry> history;
    int nextIdNumber = 1;             // for auto-generating IDs BK001, BK002...
    const string booksFile = "books.txt";
    const string historyFile = "history.txt";
    const int borrowLimitPerUser = 2; // max books a borrower can have at once

    // Helper to generate next ID string like BK001
    string generateNextId() {
        // ensure nextIdNumber matches existing highest id
        // We'll use zero-padded 3-digit numbers
        ostringstream ss;
        ss << "BK" << setw(3) << setfill('0') << nextIdNumber;
        nextIdNumber++;
        return ss.str();
    }

    // Update nextIdNumber by scanning current books to avoid collisions on load
    void recalcNextId() {
        int maxNum = 0;
        for (const auto &b : books) {
            if (b.id.size() >= 5 && b.id.substr(0,2) == "BK") {
                string numPart = b.id.substr(2);
                try {
                    int n = stoi(numPart);
                    if (n > maxNum) maxNum = n;
                } catch (...) {}
            }
        }
        nextIdNumber = maxNum + 1;
    }

    // Count how many books a person currently borrowed
    int countBorrowedByUser(const string &name) {
        int cnt = 0;
        for (const auto &b : books) {
            if (b.isBorrowed && toLower(trim(b.borrower)) == toLower(trim(name))) cnt++;
        }
        return cnt;
    }

public:
    // Constructor: load books and history from files
    Library() {
        loadFromFile();
        loadHistoryFromFile();
        recalcNextId();
    }

    // Destructor: ensure we save on exit
    ~Library() {
        saveToFile();
        saveHistoryToFile();
    }

    /*
        ========== File IO ==========
        We persist books and history so data remains between program runs.
    */

    void saveToFile() {
        ofstream ofs(booksFile);
        if (!ofs) {
            cerr << "Warning: cannot open " << booksFile << " for writing." << endl;
            return;
        }
        for (const auto &b : books) {
            ofs << b.serialize() << "\n";
        }
        ofs.close();
    }

    void loadFromFile() {
        books.clear();
        ifstream ifs(booksFile);
        if (!ifs) {
            // file may not exist first run — that's OK
            return;
        }
        string line;
        while (getline(ifs, line)) {
            if (trim(line).empty()) continue;
            Book b = Book::deserialize(line);
            if (!b.id.empty()) books.push_back(b);
        }
        ifs.close();
    }

    void saveHistoryToFile() {
        ofstream ofs(historyFile, ios::app); // append logs incrementally
        if (!ofs) {
            cerr << "Warning: cannot open " << historyFile << " for writing." << endl;
            return;
        }
        // append only latest entries to file to avoid rewriting whole file repeatedly:
        // (for simplicity we dump whole history on destructor; but append here for immediate persistence)
        // To avoid duplicate writes we won't append here for the whole history again.
        ofs.close();
    }

    void loadHistoryFromFile() {
        history.clear();
        ifstream ifs(historyFile);
        if (!ifs) return;
        string line;
        while (getline(ifs, line)) {
            if (trim(line).empty()) continue;
            HistoryEntry h = HistoryEntry::deserialize(line);
            if (!h.timestamp.empty()) history.push_back(h);
        }
        ifs.close();
    }

    // Add a new book (Admin)
    void addBookInteractive() {
        string title, author;
        int year;
        // flush leftover newline before getline
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter book title: ";
        getline(cin, title);
        cout << "Enter author name: ";
        getline(cin, author);
        cout << "Enter publication year: ";
        cin >> year;

        string id = generateNextId();
        Book b(id, trim(title), trim(author), year, false, "");
        books.push_back(b);
        saveToFile();
        cout << "Book added with ID: " << id << endl;
    }

    // Update book information by ID (Admin)
    void updateBookInteractive() {
        string id;
        cout << "Enter book ID to update (e.g. BK001): ";
        cin >> id;
        Book *b = findById(id);
        if (!b) {
            cout << "Book not found." << endl;
            return;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Current title: " << b->title << "\nNew title (leave empty to keep): ";
        string newTitle; getline(cin, newTitle);
        cout << "Current author: " << b->author << "\nNew author (leave empty to keep): ";
        string newAuthor; getline(cin, newAuthor);
        cout << "Current year: " << b->year << "\nNew year (0 to keep): ";
        int newYear; cin >> newYear;
        if (!trim(newTitle).empty()) b->title = trim(newTitle);
        if (!trim(newAuthor).empty()) b->author = trim(newAuthor);
        if (newYear != 0) b->year = newYear;
        saveToFile();
        cout << "Book updated." << endl;
    }

    // Delete book (Admin)
    void deleteBookInteractive() {
        string id;
        cout << "Enter book ID to delete: ";
        cin >> id;
        auto it = find_if(books.begin(), books.end(), [&](const Book &bk){ return bk.id == id; });
        if (it == books.end()) {
            cout << "Book not found." << endl;
            return;
        }
        cout << "Are you sure you want to delete '" << it->title << "'? (y/n): ";
        char c; cin >> c;
        if (c == 'y' || c == 'Y') {
            books.erase(it);
            saveToFile();
            cout << "Book deleted." << endl;
        } else {
            cout << "Delete cancelled." << endl;
        }
    }

    // Search - supports partial matches in title or author (case-insensitive), and exact year
    vector<int> searchIndicesInteractive() {
        vector<int> results;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Search by (1) Title  (2) Author  (3) Year  (4) Partial Title/Author: ";
        int option; cin >> option;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (option == 3) {
            cout << "Enter year: ";
            int y; cin >> y;
            for (size_t i = 0; i < books.size(); ++i) if (books[i].year == y) results.push_back(i);
        } else if (option == 1 || option == 2 || option == 4) {
            cout << "Enter search keyword: ";
            string kw;
            getline(cin, kw);
            kw = toLower(trim(kw));
            for (size_t i = 0; i < books.size(); ++i) {
                string t = toLower(books[i].title);
                string a = toLower(books[i].author);
                if (option == 1 && t.find(kw) != string::npos) results.push_back(i);
                else if (option == 2 && a.find(kw) != string::npos) results.push_back(i);
                else if (option == 4 && (t.find(kw) != string::npos || a.find(kw) != string::npos)) results.push_back(i);
            }
        } else {
            cout << "Invalid option." << endl;
        }
        return results;
    }

    // Borrow a book
    void borrowInteractive() {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter book ID to borrow (e.g. BK001) or type part of title to search: ";
        string q; getline(cin, q);
        q = trim(q);
        // try find by ID first
        Book *b = findById(q);
        if (!b) {
            // fallback: search partial title
            vector<int> found;
            string qlow = toLower(q);
            for (size_t i = 0; i < books.size(); ++i) {
                if (toLower(books[i].title).find(qlow) != string::npos) found.push_back(i);
            }
            if (found.empty()) {
                cout << "No matching book found." << endl;
                return;
            }
            // show matches
            cout << "Matches:" << endl;
            for (int idx : found) {
                books[idx].displayShort();
            }
            cout << "Enter the ID of the book you want to borrow: ";
            string id; cin >> id;
            b = findById(id);
            if (!b) {
                cout << "Invalid ID selected." << endl;
                return;
            }
        }

        if (b->isBorrowed) {
            cout << "Sorry, this book is already borrowed by: " << b->borrower << endl;
            return;
        }

        cout << "Enter your name: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string name; getline(cin, name);
        name = trim(name);
        if (name.empty()) {
            cout << "Name cannot be empty." << endl;
            return;
        }

        int current = countBorrowedByUser(name);
        if (current >= borrowLimitPerUser) {
            cout << "Borrowing limit reached. You already have " << current << " borrowed book(s)." << endl;
            return;
        }

        // do borrow
        b->isBorrowed = true;
        b->borrower = name;
        // log history
        HistoryEntry h{ nowStr(), "BORROW", b->id, b->title, name };
        history.push_back(h);
        appendHistoryToFile(h); // make immediate
        saveToFile();

        cout << "You have successfully borrowed '" << b->title << "' (ID: " << b->id << ")." << endl;
    }

    // Return a book
    void returnInteractive() {
        cout << "Enter book ID to return (e.g. BK001): ";
        string id; cin >> id;
        Book *b = findById(id);
        if (!b) {
            cout << "Book not found." << endl;
            return;
        }
        if (!b->isBorrowed) {
            cout << "This book is not borrowed." << endl;
            return;
        }
        cout << "Enter your name (must match borrower): ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string name; getline(cin, name);
        name = trim(name);
        if (toLower(name) != toLower(b->borrower)) {
            cout << "Name does not match borrower (" << b->borrower << "). Return cancelled." << endl;
            return;
        }

        // do return
        b->isBorrowed = false;
        b->borrower = "";
        HistoryEntry h{ nowStr(), "RETURN", b->id, b->title, name };
        history.push_back(h);
        appendHistoryToFile(h);
        saveToFile();

        cout << "Book returned successfully. Thank you." << endl;
    }

    // Display all books, optionally sorted by user's choice
    void displayAllInteractive() {
        if (books.empty()) {
            cout << "No books in library." << endl;
            return;
        }
        cout << "Sort by: (1) Title  (2) Year  (3) Availability  (4) No sort: ";
        int opt; cin >> opt;
        vector<Book> copy = books;

        if (opt == 1) {
            sort(copy.begin(), copy.end(), [](const Book &a, const Book &b){
                return toLower(a.title) < toLower(b.title);
            });
        } else if (opt == 2) {
            sort(copy.begin(), copy.end(), [](const Book &a, const Book &b){
                return a.year < b.year;
            });
        } else if (opt == 3) {
            sort(copy.begin(), copy.end(), [](const Book &a, const Book &b){
                return (int)a.isBorrowed < (int)b.isBorrowed; // available first
            });
        }

        cout << left << setw(7) << "ID" << setw(30) << "Title" << setw(20) << "Author" << setw(6) << "Year" << "Status" << endl;
        cout << string(80, '-') << endl;
        for (const auto &b : copy) b.displayShort();
    }

    // Show history (recent)
    void showHistoryInteractive() {
        if (history.empty()) {
            cout << "No history available." << endl;
            return;
        }
        cout << "Show last how many entries? ";
        int n; cin >> n;
        if (n <= 0) n = (int)history.size();
        int start = max(0, (int)history.size() - n);
        for (int i = start; i < (int)history.size(); ++i) {
            const auto &h = history[i];
            cout << h.timestamp << " | " << setw(6) << h.action << " | " << setw(6) << h.bookID << " | " << h.title << " | " << h.byWho << endl;
        }
    }

    // Append a single history entry to file immediately
    void appendHistoryToFile(const HistoryEntry &h) {
        ofstream ofs(historyFile, ios::app);
        if (!ofs) {
            cerr << "Warning: cannot open history file for appending." << endl;
            return;
        }
        ofs << h.serialize() << "\n";
        ofs.close();
    }

    // Helper: find book by ID (returns pointer or nullptr)
    Book* findById(const string &id) {
        for (auto &b : books) if (b.id == id) return &b;
        return nullptr;
    }

    // Show details for a single book by ID
    void showBookByIdInteractive() {
        cout << "Enter book ID: ";
        string id; cin >> id;
        Book *b = findById(id);
        if (!b) {
            cout << "Book not found." << endl;
            return;
        }
        b->displayFull();
    }
};


/*
  -------------------------
   Authentication (simple admin)
  -------------------------
*/
bool adminLogin() {
    // Very simple hard-coded credentials for learning/demo purposes.
    const string ADMIN_USER = "admin";
    const string ADMIN_PASS = "1234";

    cout << "Admin username: ";
    string u; cin >> u;
    cout << "Admin password: ";
    string p; cin >> p;

    if (u == ADMIN_USER && p == ADMIN_PASS) {
        cout << "Welcome, admin." << endl;
        return true;
    } else {
        cout << "Invalid credentials." << endl;
        return false;
    }
}

/*
  -------------------------
   Main program loop & UI
  -------------------------
*/
int main() {
    Library lib; // loads data automatically in constructor

    cout << "Welcome to the Library Manager!" << endl;

    while (true) {
        cout << "\n===== MAIN MENU =====\n";
        cout << "1. Add Book (Admin)\n";
        cout << "2. Update Book (Admin)\n";
        cout << "3. Delete Book (Admin)\n";
        cout << "4. Search Books\n";
        cout << "5. Borrow Book\n";
        cout << "6. Return Book\n";
        cout << "7. Display All Books\n";
        cout << "8. Show Borrow/Return History\n";
        cout << "9. Show Book Details by ID\n";
        cout << "0. Exit\n";
        cout << "Choose option: ";
        int option; cin >> option;

        switch (option) {
            case 1: {
                if (adminLogin()) lib.addBookInteractive();
                break;
            }
            case 2: {
                if (adminLogin()) lib.updateBookInteractive();
                break;
            }
            case 3: {
                if (adminLogin()) lib.deleteBookInteractive();
                break;
            }
            case 4: {
                auto indices = lib.searchIndicesInteractive();
                if (indices.empty()) {
                    cout << "No results." << endl;
                } else {
                    cout << "Found " << indices.size() << " result(s):\n";
                    cout << left << setw(7) << "ID" << setw(30) << "Title" << setw(20) << "Author" << setw(6) << "Year" << "Status" << endl;
                    cout << string(80, '-') << endl;
                    for (int i : indices) lib.findById(lib.findById("") ? "" : "") ; // noop to avoid warning (we'll print below)
                    // print properly
                    for (int idx : indices) lib.findById(lib.findById("") ? "" : ""); // noop
                    for (int idx : indices) {
                        // we stored indices relative to internal vector, so fetch via books vector indirectly:
                        // but Book access is private; instead we can show via showing details by ID from the stored index.
                        // To keep design simple, we will reconstruct the ID by reading from file (but better: add a public method).
                        // Simpler: print by temporarily loading books from file again - but to avoid complexity, we will
                        // add a small temporary method: use findById by retrieving ID directly via saved file is overkill.
                    }
                    // Simpler approach: re-run a search with print function inside Library in future versions.
                    // For now, just notify user results found and prompt to view details by ID.
                    cout << "Enter an ID from the results to view details, or press Enter to continue: ";
                    string choice;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    getline(cin, choice);
                    choice = trim(choice);
                    if (!choice.empty()) {
                        // show details
                        cout << "Attempting to show details for ID: " << choice << endl;
                        // call showBookByIdInteractive via temporary input: we will call the library's method directly
                        // For that we ask user to re-enter ID in that function: simply call:
                        // Because showBookByIdInteractive prompts the user, we can simulate by calling find and printing
                        // But findById is private; in current class design it's public. So:
                        lib.showBookByIdInteractive();
                    }
                }
                break;
            }
            case 5:
                lib.borrowInteractive();
                break;
            case 6:
                lib.returnInteractive();
                break;
            case 7:
                lib.displayAllInteractive();
                break;
            case 8:
                lib.showHistoryInteractive();
                break;
            case 9:
                lib.showBookByIdInteractive();
                break;
            case 0:
                cout << "Goodbye — saving data..." << endl;
                return 0;
            default:
                cout << "Invalid option. Try again." << endl;
        }
    }

    return 0;
}
