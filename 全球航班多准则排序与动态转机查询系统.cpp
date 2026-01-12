#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <stack>
#include <climits>
#include <map>

using namespace std;

// ==========================================
// 1. 数据结构定义
// ==========================================

// 航班信息结构体
struct Flight {
    string flightNo;  // 航班号
    string startCity; // 起点
    string endCity;   // 终点
    double price;     // 票价
    int duration;     // 飞行时长(分钟)
    double onTimeRate;// 准点率 (0.0 - 1.0)
};

// ==========================================
// 2. 航班管理与排序模块 (线性表)
// ==========================================

class FlightManager {
public:
    vector<Flight> flights;

    // 添加航班
    void addFlight(string no, string s, string e, double p, int d, double r) {
        flights.push_back({no, s, e, p, d, r});
    }

    // 显示所有航班
    void display() {
        cout << string(80, '-') << endl;
        cout << left << setw(10) << "航班号" << setw(15) << "起点" << setw(15) << "终点"
             << setw(10) << "票价" << setw(10) << "时长(分)" << setw(10) << "准点率" << endl;
        cout << string(80, '-') << endl;
        for (const auto& f : flights) {
            cout << left << setw(10) << f.flightNo << setw(15) << f.startCity << setw(15) << f.endCity
                 << setw(10) << f.price << setw(10) << f.duration << setw(10) << f.onTimeRate << endl;
        }
        cout << endl;
    }

    // --- A. 冒泡排序 (按票价) ---
    void bubbleSortByPrice() {
        int n = flights.size();
        for (int i = 0; i < n - 1; ++i) {
            for (int j = 0; j < n - i - 1; ++j) {
                if (flights[j].price > flights[j + 1].price) {
                    swap(flights[j], flights[j + 1]);
                }
            }
        }
        cout << "[提示] 已按票价完成冒泡排序。\n";
    }

    // --- B. 快速排序 (按时长) ---
    int partition(int low, int high) {
        Flight pivot = flights[high];
        int i = (low - 1);
        for (int j = low; j <= high - 1; j++) {
            if (flights[j].duration < pivot.duration) {
                i++;
                swap(flights[i], flights[j]);
            }
        }
        swap(flights[i + 1], flights[high]);
        return (i + 1);
    }

    void quickSortByDuration(int low, int high) {
        if (low < high) {
            int pi = partition(low, high);
            quickSortByDuration(low, pi - 1);
            quickSortByDuration(pi + 1, high);
        }
    }

    // --- C. 堆排序 (按准点率) ---
    void heapify(int n, int i) {
        int largest = i;
        int l = 2 * i + 1;
        int r = 2 * i + 2;

        if (l < n && flights[l].onTimeRate > flights[largest].onTimeRate)
            largest = l;
        if (r < n && flights[r].onTimeRate > flights[largest].onTimeRate)
            largest = r;

        if (largest != i) {
            swap(flights[i], flights[largest]);
            heapify(n, largest);
        }
    }

    void heapSortByRate() {
        int n = flights.size();
        for (int i = n / 2 - 1; i >= 0; i--)
            heapify(n, i);
        for (int i = n - 1; i > 0; i--) {
            swap(flights[0], flights[i]);
            heapify(i, 0);
        }
        // 注意：标准堆排序后通常是从小到大，若需降序(准点率高在前)可反转或调整比较逻辑
        // 这里演示标准堆排序逻辑
        cout << "[提示] 已按准点率完成堆排序。\n";
    }

    // --- D. 基数排序 (按航班号) ---
    // 假设航班号格式固定，如 "CA123"，简单演示对最后一位数字的基数排序逻辑
    // 实际项目中通常对字符串进行 LSD (Least Significant Digit) 排序
    void radixSortByFlightNo() {
        // 获取最大长度
        size_t maxLen = 0;
        for(auto& f : flights) maxLen = max(maxLen, f.flightNo.length());

        // 对每一位字符进行计数排序
        for(int pos = maxLen - 1; pos >= 0; pos--) {
            vector<Flight> output(flights.size());
            int count[256] = {0};

            for(size_t i = 0; i < flights.size(); i++) {
                int charIndex = (pos < flights[i].flightNo.length()) ? (unsigned char)flights[i].flightNo[pos] : 0;
                count[charIndex]++;
            }

            for(int i = 1; i < 256; i++) count[i] += count[i - 1];

            for(int i = flights.size() - 1; i >= 0; i--) {
                int charIndex = (pos < flights[i].flightNo.length()) ? (unsigned char)flights[i].flightNo[pos] : 0;
                output[count[charIndex] - 1] = flights[i];
                count[charIndex]--;
            }
            flights = output;
        }
        cout << "[提示] 已按航班号完成基数排序。\n";
    }
};

// ==========================================
// 3. 航线图与路径咨询模块 (图结构)
// ==========================================

const int MAX_CITIES = 20;
const double INF = 1e9;

class RouteGraph {
private:
    int numVertices;
    string cityNames[MAX_CITIES];
    double adjMatrix[MAX_CITIES][MAX_CITIES]; // 存储票价作为权值
    map<string, int> cityToIndex;
    bool visited[MAX_CITIES];
    vector<int> pathStack;

public:
    RouteGraph() {
        numVertices = 0;
        for(int i=0; i<MAX_CITIES; ++i)
            for(int j=0; j<MAX_CITIES; ++j)
                adjMatrix[i][j] = (i==j ? 0 : INF);
    }

    void addCity(string name) {
        if(cityToIndex.find(name) == cityToIndex.end()) {
            cityNames[numVertices] = name;
            cityToIndex[name] = numVertices;
            numVertices++;
        }
    }

    void addRoute(string from, string to, double price) {
        addCity(from);
        addCity(to);
        int u = cityToIndex[from];
        int v = cityToIndex[to];
        // 保留最低票价作为边权（如果有多个航班）
        if(price < adjMatrix[u][v]) {
            adjMatrix[u][v] = price;
        }
    }

    int getCityIndex(string name) {
        if(cityToIndex.find(name) != cityToIndex.end()) return cityToIndex[name];
        return -1;
    }

    // --- Dijkstra 算法：最低票价最短路径 ---
    void dijkstra(string startCity, string endCity) {
        int start = getCityIndex(startCity);
        int end = getCityIndex(endCity);

        if(start == -1 || end == -1) {
            cout << "错误：城市不存在。\n";
            return;
        }

        double dist[MAX_CITIES];
        bool visitedNode[MAX_CITIES];
        int parent[MAX_CITIES];

        for(int i=0; i<numVertices; ++i) {
            dist[i] = INF;
            visitedNode[i] = false;
            parent[i] = -1;
        }

        dist[start] = 0;

        for(int i=0; i<numVertices; ++i) {
            int u = -1;
            double minDist = INF;
            // 找最小dist的未访问节点
            for(int j=0; j<numVertices; ++j) {
                if(!visitedNode[j] && dist[j] < minDist) {
                    minDist = dist[j];
                    u = j;
                }
            }

            if(u == -1 || dist[u] == INF) break;
            visitedNode[u] = true;

            for(int v=0; v<numVertices; ++v) {
                if(!visitedNode[v] && adjMatrix[u][v] != INF) {
                    if(dist[u] + adjMatrix[u][v] < dist[v]) {
                        dist[v] = dist[u] + adjMatrix[u][v];
                        parent[v] = u;
                    }
                }
            }
        }

        if(dist[end] == INF) {
            cout << startCity << " 到 " << endCity << " 无可行路径。\n";
        } else {
            cout << ">>> 最优路径 (最低票价): " << dist[end] << " 元\n";
            cout << "路径: ";
            vector<int> path;
            int curr = end;
            while(curr != -1) {
                path.push_back(curr);
                curr = parent[curr];
            }
            for(int i=path.size()-1; i>=0; --i) {
                cout << cityNames[path[i]] << (i > 0 ? " -> " : "");
            }
            cout << endl;
        }
    }

    // --- DFS: 查找所有简单路径 ---
    void findAllPaths(int u, int d, double currentDist) {
        visited[u] = true;
        pathStack.push_back(u);

        if(u == d) {
            cout << "路径: ";
            for(size_t i=0; i<pathStack.size(); ++i) {
                cout << cityNames[pathStack[i]] << (i < pathStack.size()-1 ? " -> " : "");
            }
            cout << " | 总票价: " << currentDist << endl;
        } else {
            for(int v=0; v<numVertices; ++v) {
                if(adjMatrix[u][v] != INF && !visited[v]) {
                    findAllPaths(v, d, currentDist + adjMatrix[u][v]);
                }
            }
        }

        pathStack.pop_back();
        visited[u] = false; // 回溯
    }

    void searchAllPaths(string start, string end) {
        int u = getCityIndex(start);
        int v = getCityIndex(end);
        if(u == -1 || v == -1) { cout << "城市不存在。\n"; return; }

        for(int i=0; i<MAX_CITIES; ++i) visited[i] = false;
        pathStack.clear();

        cout << ">>> " << start << " 到 " << end << " 的所有简单路径:\n";
        findAllPaths(u, v, 0);
    }
};

// ==========================================
// 4. 主程序与菜单
// ==========================================

void loadSampleData(FlightManager &fm, RouteGraph &rg) {
    // 模拟15+个城市的航班数据
    struct Data { string no, s, e; double p; int d; double r; };
    vector<Data> raw = {
        {"CA101", "Beijing", "Shanghai", 1200, 130, 0.95},
        {"MU505", "Shanghai", "Tokyo", 2500, 180, 0.88},
        {"JL789", "Tokyo", "NewYork", 8000, 720, 0.92},
        {"AA100", "NewYork", "London", 4500, 400, 0.85},
        {"BA202", "London", "Paris", 800, 90, 0.90},
        {"AF303", "Paris", "Berlin", 600, 100, 0.93},
        {"LH404", "Berlin", "Moscow", 1500, 200, 0.89},
        {"SU505", "Moscow", "Beijing", 3000, 480, 0.87},
        {"CZ606", "Beijing", "Guangzhou", 1800, 190, 0.91},
        {"CX707", "Guangzhou", "HongKong", 500, 50, 0.96},
        {"SQ808", "HongKong", "Singapore", 2000, 240, 0.94},
        {"QF909", "Singapore", "Sydney", 3500, 450, 0.90},
        {"NZ001", "Sydney", "Auckland", 1200, 180, 0.88},
        {"UA111", "Auckland", "LosAngeles", 6000, 700, 0.86},
        {"DL222", "LosAngeles", "NewYork", 2200, 300, 0.89},
        {"CA102", "Beijing", "Tokyo", 2800, 200, 0.91} // 额外路径
    };

    for(auto& d : raw) {
        fm.addFlight(d.no, d.s, d.e, d.p, d.d, d.r);
        rg.addRoute(d.s, d.e, d.p);
    }
}

int main() {
    FlightManager fm;
    RouteGraph rg;

    // 初始化数据
    loadSampleData(fm, rg);

    int choice;
    while(true) {
        cout << "\n========== 全球航班查询系统 ==========\n";
        cout << "1. 显示所有航班\n";
        cout << "2. 按票价排序 (冒泡)\n";
        cout << "3. 按时长排序 (快排)\n";
        cout << "4. 按准点率排序 (堆排)\n";
        cout << "5. 按航班号排序 (基数)\n";
        cout << "6. 查询两地所有路径 (DFS)\n";
        cout << "7. 查询最低票价路径 (Dijkstra)\n";
        cout << "0. 退出\n";
        cout << "请输入选择: ";
        cin >> choice;

        if(choice == 0) break;

        string s, e;
        switch(choice) {
            case 1: fm.display(); break;
            case 2: fm.bubbleSortByPrice(); fm.display(); break;
            case 3: fm.quickSortByDuration(0, fm.flights.size()-1); fm.display(); break;
            case 4: fm.heapSortByRate(); fm.display(); break;
            case 5: fm.radixSortByFlightNo(); fm.display(); break;
            case 6:
                cout << "输入起点 终点 (如 Beijing Tokyo): ";
                cin >> s >> e;
                rg.searchAllPaths(s, e);
                break;
            case 7:
                cout << "输入起点 终点 (如 Beijing Tokyo): ";
                cin >> s >> e;
                rg.dijkstra(s, e);
                break;
            default: cout << "无效输入\n";
        }
    }
    return 0;
}