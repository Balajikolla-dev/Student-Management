# Student Management CRUD Storage Engine

A high-performance, lightweight Student Record Management System engineered in C++ for Linux systems. The core storage engine utilizes raw file I/O operations on binary fixed-width structures combined with an in-memory hash-map index. This design achieves **O(1) time complexity** for lookups, modifications, and deletions with a **Zero Data Loss** hardware-flushing guarantee.

---
## Core Features & Mechanics

* **O(1) Fast Operations:** By storing raw data file stream offsets (`std::streampos`) within a memory-mapped lookup index, the engine skips linear iterative scans completely. It jumps directly to specific file locations instantly.
* **Deterministic Memory Footprint:** Built on an unpadded fixed-width structural matrix (`#pragma pack(push, 1)`), enforcing a strict layout size of exactly **89 bytes** per student entry on disk.
* **In-Place Updates:** Modifying structural criteria maps instantly onto existing sector offsets, patching target bits without displacing neighboring records or corrupting files.
* **Zero Data Loss Guarantee:** Overrides standard lazy operating system caching configurations by triggering `file.flush()` instantly after every database mutation, protecting data against application crashes or sudden power losses.
* **Instant Soft-Deletions:** Drops targets from tracking maps and flips an internal boolean state marker on disk. This avoids shifting kilobytes of subsequent blocks and maintains system performance.
* **Excel Integration:** Includes a dedicated serialization utility that loops through active storage blocks to dump clean, standard `.csv` outputs readable by Microsoft Excel or LibreOffice Calc.

---

## Module Pipeline Flow

1. **Instantiation:** On boot, the constructor attempts a non-destructive read/write link to `students.dat`. If missing, it creates the file.
2. **Rebuild Index Pass:** The engine parses the data sequentially *exactly once* at startup. Valid active entries have their precise file track addresses mapped directly to memory.
3. **Write Path:** New keys are checked for uniqueness against the index map, appended to the file end, flushed to disk, and saved to the runtime memory index.
4. **Update/Delete Path:** The target ID maps to a file address, the engine positions its write pointer (`seekp`), alters the state fields, and commits the data instantly.

---

## File Structure Representation

Every item maps tightly across the following structural layout:

| Field Element | Native Type | Disk Boundary Constraint | Size Allocation |
| :--- | :--- | :--- | :--- |
| **ID** | `int` | Primary Key Unique Field | 4 Bytes |
| **Name** | `char[50]` | Static string storage buffer | 50 Bytes |
| **Department** | `char[30]` | Static string storage buffer | 30 Bytes |
| **GPA** | `float` | Fractional metric balance | 4 Bytes |
| **Active Flag** | `bool` | Soft-deletion execution marker | 1 Byte |
| **Total Block Width** | | | **89 Bytes** |

---

## Compiling and Running the Engine

The engine is built to run natively across modern Linux setups (like Linux Mint or Ubuntu) utilizing the GNU Compiler Collection (`g++`).

### 1. Build the Binary
Compile the implementation file using the aggressive `-O3` flag optimization profile:

g++ -std=c++17 -O3 main.cpp -o main

---

### 2. Launch the Application

Execute the compiled binary code runner:
Bash

./main

---

### 3. Review Generated Outputs

You can check the storage file and generated spreadsheet reports via the terminal layout tools:
#### Check the exact byte size of your binary database
ls -lh students.dat

#### Inspect the Excel-ready CSV file format export 
cat student_report.csv
