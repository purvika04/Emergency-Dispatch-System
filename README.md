# Emergency Dispatch System 🚨

A comprehensive, real-time **Emergency Dispatch System** that combines a responsive **Flask Web Interface** with a high-performance **C backend**. This system demonstrates key data structures and algorithms—specifically **Min-Heaps** for priority queueing and **Dijkstra's Shortest Path Algorithm** for navigation routing.

---

## 🛠️ Tech Stack & Key Concepts

1. **Frontend**: HTML5, Vanilla CSS3 (modern glassmorphic design), and JavaScript (dynamic API fetching and UI state management).
2. **Web Controller (Middleware)**: Python Flask (`app.py`), bridging the web frontend with the compiled C executable via subprocess.
3. **C Backend (`Untitled-1.c`)**:
   - **Min-Heap (Priority Queue)**: Manages incoming emergencies. Incidents are sorted dynamically by a custom priority score (calculated using incident severity).
   - **Graph Representation**: Map represented as an Adjacency List.
   - **Dijkstra's Algorithm**: Calculates the shortest path from the active dispatch unit to the emergency site.
   - **State Management**: Serializes and deserializes simulator status to `system_state.bin` on disk.

---

## 📂 Project Structure

```bash
├── app.py                  # Python Flask web server
├── Untitled-1.c            # C backend source code (Algorithms & State Management)
├── emergency_system        # Compiled C executable for macOS/Linux
├── emergency_system.exe    # Compiled C executable for Windows
├── templates/
│   └── index.html          # Web dashboard layout
├── static/
│   ├── style.css           # UI Styling (responsive dashboard layout)
│   └── script.js           # AJAX operations, state polling & updates
└── README.md               # Documentation
```

---

## 🚀 How to Run the Application

### 1. Install Dependencies
Ensure you have Python 3 and Flask installed:
```bash
pip install flask
```

### 2. Compile the C Backend (Optional)
The pre-compiled cross-platform binaries are already provided in the repository. If you make modifications to the C source code (`Untitled-1.c`), you can recompile it:

*   **macOS / Linux**:
    ```bash
    gcc Untitled-1.c -o emergency_system
    ```
*   **Windows (MinGW/GCC)**:
    ```bash
    gcc Untitled-1.c -o emergency_system.exe
    ```

### 3. Start the Web Server
Launch the Flask development server:
```bash
python app.py
```
By default, the application will run at **`http://127.0.0.1:8000`**. Open this URL in your web browser to access the interactive dashboard.

---

## ⚙️ Features & Workflow

*   **Report Emergency**: File new incidents specifying Severity (1-5), Location Node (0-19), and Incident Type (Medical / Fire / Rescue). The C backend pushes the incident into a Min-Heap.
*   **Smart Dispatching**: Dispatches the closest available unit matching the required type (Ambulance or Firetruck) using Dijkstra's algorithm.
*   **Visual Dispatch Log**: Displays real-time status of all emergency units, current tasks, and transit paths.
*   **Simulation Control**: Easy controls to manually complete unit assignments and reset the simulation state back to base.
