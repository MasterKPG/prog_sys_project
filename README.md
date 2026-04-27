# Multi-Process Car Parking System

A Linux-based multi-process car parking simulator built as part of the Systems Programming course at **ENSSAT, Université de Rennes**. The system simulates a real-world parking lot using inter-process communication (IPC) mechanisms provided by the Linux kernel.

---

## 📐 Architecture

The application follows a **client-server model**:

- The **server** is the central parking manager. It runs continuously, maintains the parking state in shared memory, and processes client requests sequentially.
- Each **client** represents a car requesting to enter or exit the parking lot. It communicates with the server through named pipes (FIFOs) and waits for a response.
- The **inspector** is an independent monitoring process that can read and display the current parking state at any time.

### Communication Flow

```
Client 1 ──┐                        ┌──► Answer Pipe (PID 1) ──► Client 1
Client 2 ──┼──► Requests Pipe ──► Server ──► Answer Pipe (PID 2) ──► Client 2
Client 3 ──┘                        └──► Answer Pipe (PID 3) ──► Client 3
                                         │
                                    Shared Memory
                                    (Parking State)
                                         │
                                      Inspector
```

- **1 shared requests pipe** — all clients write to it, server reads from it
- **1 dedicated answer pipe per client** — named using the client's PID
- **1 shared memory segment** — stores the parking state (`capacity`, `num_cars`, `car_ids[]`)

---

## 📁 Project Structure

```
.
├── structures.h       # Shared data structures and constants
├── structures.c       # Utility functions (print_request, print_response)
├── server.c           # Server implementation
├── client.c           # Client implementation
├── inspector.c        # Independent parking state monitor
├── logger.h           # Logger interface (provided)
├── logger.o           # Pre-compiled logger (provided)
├── Makefile           # Build system
└── README.md          # This file
```

---

## 🔧 Build

```bash
make
```

To clean build artifacts:

```bash
make clean
```

---

## 🚀 Usage

### 1. Start the server
```bash
./server <capacity>
```
Example — start a parking lot with 5 spots:
```bash
./server 5
```

### 2. Send client requests (in separate terminals)
```bash
./client <car_id> <action>
```
Examples:
```bash
./client 007 enter
./client 42 enter
./client 007 exit
```

### 3. Inspect the parking state (at any time)
```bash
./inspector
```

### 4. Multiple simultaneous clients
```bash
./client 001 enter & ./client 002 enter & ./client 003 enter
```

---

## 📦 Data Structures

| Structure | Description |
|---|---|
| `request_t` | Client request: `client_pid`, `car_id`, `action` (ENTER/EXIT) |
| `response_t` | Server response: `car_id`, `status` |
| `parking_state_t` | Parking state: `capacity`, `num_cars`, `car_ids[]` |

### Response Status Codes

| Status | Description |
|---|---|
| `SUCCESS` | Operation completed successfully |
| `FULL` | Parking lot is full (entry denied) |
| `NOT_FOUND` | Car not found in parking (exit denied) |
| `ALREADY_PARKED` | Car is already in the parking lot |

---

## ✅ Implementation Progress

### March — Basic Version
- [x] **Task 1** — Data structures (`request_t`, `response_t`, `parking_state_t`)
- [x] **Task 2** — Basic client/server implementation with named pipes and shared memory
- [x] **Task 3** — SIGINT signal handler for graceful server shutdown
- [x] **Task 4** — Semaphore-based synchronization for shared memory access
- [x] **Task 5** — Log monitoring using the provided logger module
- [ ] **Task 6** — Secure log file access with `mylogger` wrapper module

### May — New Logger Version
- [ ] **Task 7** — Handle logger failures by delegating log writes to child processes
  
### July — Multi-Thread Version
- [ ] **Task 8** — Proof of concept: thread-based logger failure handling
- [ ] **Task 9** — Comparison report: multi-process vs multi-thread approaches

---

## 🛠️ Implementation Details

### Named Pipes (FIFOs)
- Requests pipe: `/tmp/parking_requests` (fixed, defined in `structures.h`)
- Answer pipes: `/tmp/parking_answer_<PID>` (dynamic, one per client)

### Shared Memory
- Name: `/parking_lot_shm` (defined in `structures.h`)
- Created fresh at server startup, unlinked on shutdown
- Contains the full `parking_state_t` structure

### Signal Handling
- The server handles `SIGINT` (Ctrl+C) gracefully
- On shutdown: closes all open file descriptors, unlinks the requests pipe, and unlinks the shared memory segment

---

## 👤 Author

Mohammad Amara @ ENSSAT, Informatique 1, Université de Rennes
