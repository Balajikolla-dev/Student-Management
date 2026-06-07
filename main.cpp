#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <limits>

#pragma pack(push, 1) 
struct Student {
    int id;
    char name[50];
    char department[30];
    float gpa;
    bool is_active;  
};
#pragma pack(pop)

class StudentCrudEngine {
private:
    std::string filename;
    std::fstream file;
    std::unordered_map<int, std::streampos> index_table;

    void rebuild_index() {
        index_table.clear();
        file.seekg(0, std::ios::beg);
        
        Student s;
        std::streampos current_pos = file.tellg();
        
        while (file.read(reinterpret_cast<char*>(&s), sizeof(Student))) {
            if (s.is_active) {
                index_table[s.id] = current_pos;
            }
            current_pos = file.tellg();
        }
        file.clear(); 
    }

public:
    StudentCrudEngine(std::string db_file) : filename(db_file) {
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!file) {
            file.open(filename, std::ios::out | std::ios::binary);
            file.close();
            file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        }
        rebuild_index();
    }

    ~StudentCrudEngine() {
        if (file.is_open()) {
            file.close();
        }
    }

    bool create_record(const Student& s) {
        if (index_table.find(s.id) != index_table.end()) {
            return false; 
        }
        file.seekp(0, std::ios::end);
        std::streampos write_pos = file.tellp();
        
        file.write(reinterpret_cast<const char*>(&s), sizeof(Student));
        file.flush(); 

        index_table[s.id] = write_pos;
        return true;
    }

    bool read_record(int id, Student& s) {
        auto it = index_table.find(id);
        if (it == index_table.end()) return false; 

        file.seekg(it->second, std::ios::beg);
        file.read(reinterpret_cast<char*>(&s), sizeof(Student));
        return true;
    }

    bool update_record(int id, const Student& updated_student) {
        auto it = index_table.find(id);
        if (it == index_table.end()) return false;

        file.seekp(it->second, std::ios::beg);
        file.write(reinterpret_cast<const char*>(&updated_student), sizeof(Student));
        file.flush(); 
        return true;
    }

    bool delete_record(int id) {
        auto it = index_table.find(id);
        if (it == index_table.end()) return false;

        Student s;
        file.seekg(it->second, std::ios::beg);
        file.read(reinterpret_cast<char*>(&s), sizeof(Student));
        
        s.is_active = false; 
        
        file.seekp(it->second, std::ios::beg);
        file.write(reinterpret_cast<char*>(&s), sizeof(Student));
        file.flush();

        index_table.erase(it); 
        return true;
    }

    void export_to_excel(const std::string& csv_filename) {
        std::ofstream csv_file(csv_filename);
        csv_file << "ID,Name,Department,GPA\n"; 

        file.seekg(0, std::ios::beg);
        Student s;
        while (file.read(reinterpret_cast<char*>(&s), sizeof(Student))) {
            if (s.is_active) {
                csv_file << s.id << "," 
                         << s.name << "," 
                         << s.department << "," 
                         << s.gpa << "\n";
            }
        }
        csv_file.flush();
    }
};

void clear_input_buffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
    StudentCrudEngine db("students.dat");
    int choice;

    while (true) {
        std::cout << "\n====================================\n";
        std::cout << "    STUDENT CRUD STORAGE ENGINE     \n";
        std::cout << "====================================\n";
        std::cout << "1. Create Student Record\n";
        std::cout << "2. Read Student Record\n";
        std::cout << "3. Update Student In-Place\n";
        std::cout << "4. Delete Student\n";
        std::cout << "5. Export Database to Excel\n";
        std::cout << "6. Exit Application\n";
        std::cout << "Enter selection (1-6): ";
        
        if (!(std::cin >> choice)) {
            std::cout << "Invalid numeric option.\n";
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        if (choice == 6) {
            std::cout << "Safely closing engine streams. Goodbye!\n";
            break;
        }

        switch (choice) {
            case 1: { 
                Student s;
                s.is_active = true;
                std::cout << "Enter unique Student ID: ";
                std::cin >> s.id;
                clear_input_buffer();

                std::cout << "Enter Name: ";
                std::cin.getline(s.name, sizeof(s.name));

                std::cout << "Enter Department: ";
                std::cin.getline(s.department, sizeof(s.department));

                std::cout << "Enter GPA: ";
                std::cin >> s.gpa;
                clear_input_buffer();

                if (db.create_record(s)) {
                    std::cout << ">> Success: Record written securely to file storage.\n";
                } else {
                    std::cout << ">> Error: Primary ID violation. Data rejected.\n";
                }
                break;
            }
            case 2: { 
                int id;
                Student s;
                std::cout << "Enter Student ID to lookup: ";
                std::cin >> id;
                clear_input_buffer();

                if (db.read_record(id, s)) {
                    std::cout << "\nID: " << s.id << "\nName: " << s.name 
                              << "\nDept: " << s.department << "\nGPA: " << s.gpa << "\n";
                } else {
                    std::cout << ">> Error: Entry not found or inactive.\n";
                }
                break;
            }
            case 3: { 
                int id;
                Student s;
                std::cout << "Enter target Student ID to modify: ";
                std::cin >> id;
                clear_input_buffer();

                if (!db.read_record(id, s)) {
                    std::cout << ">> Error: Target entry doesn't exist.\n";
                    break;
                }

                std::cout << "Modifying record for ID " << id << "...\n";
                std::cout << "Enter New Name: ";
                std::cin.getline(s.name, sizeof(s.name));
                std::cout << "Enter New Department: ";
                std::cin.getline(s.department, sizeof(s.department));
                std::cout << "Enter New GPA: ";
                std::cin >> s.gpa;
                clear_input_buffer();

                if (db.update_record(id, s)) {
                    std::cout << ">> Success: Fixed-width row updated in-place.\n";
                } else {
                    std::cout << ">> Error updating entry.\n";
                }
                break;
            }
            case 4: { 
                int id;
                std::cout << "Enter Student ID to soft-delete: ";
                std::cin >> id;
                clear_input_buffer();

                if (db.delete_record(id)) {
                    std::cout << ">> Success: Entry flagged inactive & memory index dropped.\n";
                } else {
                    std::cout << ">> Error: Target entry not found.\n";
                }
                break;
            }
            case 5: { 
                std::string csv_name = "student_report.csv";
                db.export_to_excel(csv_name);
                std::cout << ">> Success: Active rows dumped to " << csv_name << ".\n";
                break;
            }
            default:
                std::cout << "Invalid instruction choice.\n";
        }
    }
    return 0;
}
