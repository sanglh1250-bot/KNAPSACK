#include <iostream>
#include <vector>       
#include <algorithm>    
#include <chrono>      
#include <numeric>     
#include <iomanip>      

using namespace std;
using namespace std::chrono;

struct ItemSimple {
    int val;
    int wt; 
};

struct ItemBB {
    int val;
    int wt;
    double ratio;
};


bool compareItems(ItemBB a, ItemBB b) {
    return a.ratio > b.ratio;
}

int knapsack_BruteForce(int W, const vector<ItemSimple>& items) {
    int n = items.size();
    int max_value = 0;
    
    long long num_subsets = 1LL << n;

    for (long long i = 0; i < num_subsets; ++i) {
        int current_weight = 0;
        int current_value = 0;

        for (int j = 0; j < n; ++j) {
            if ((i >> j) & 1) {
                current_weight += items[j].wt;
                current_value += items[j].val;
            }
        }

        if (current_weight <= W) {
            max_value = max(max_value, current_value);
        }
    }
    return max_value;
}


void backtrack_helper(int index, int current_weight, int current_value,
                      int& max_value, int W, const vector<ItemSimple>& items) {
    int n = items.size();

    if (index == n) {
        max_value = max(max_value, current_value);
        return;
    }

    backtrack_helper(index + 1, current_weight, current_value, max_value, W, items);

    if (current_weight + items[index].wt <= W) {
        backtrack_helper(index + 1,
                         current_weight + items[index].wt,
                         current_value + items[index].val,
                         max_value, W, items);
    }
}

int knapsack_Backtracking(int W, const vector<ItemSimple>& items) {
    int max_value = 0;
    backtrack_helper(0, 0, 0, max_value, W, items);
    return max_value;
}

int knapsack_DP(int W, const vector<ItemSimple>& items) {
    int n = items.size();
    
    vector<int> dp(W + 1, 0);

    for (int i = 0; i < n; ++i) {
        for (int j = W; j >= items[i].wt; --j) {
            dp[j] = max(dp[j], dp[j - items[i].wt] + items[i].val);
        }
    }
    
    return dp[W];
}

double calculate_bound(int index, int current_weight, int current_value,
                       int W, const vector<ItemBB>& items) {
    int n = items.size();
    int w = current_weight;
    double v = current_value;

    for (int i = index; i < n; ++i) {
        if (w + items[i].wt <= W) {
            w += items[i].wt;
            v += items[i].val;
        } else {
            double remaining_w = W - w;
            v += items[i].ratio * remaining_w;
            break;
        }
    }
    return v;
}

void bb_helper(int index, int current_weight, int current_value,
               double& max_value, int W, const vector<ItemBB>& items) {
    int n = items.size();

    if (current_weight <= W && current_value > max_value) {
        max_value = (double)current_value;
    }

    if (index == n) {
        return;
    }

    double bound = calculate_bound(index, current_weight, current_value, W, items);

    if (bound <= max_value) {
        return;
    }

    if (current_weight + items[index].wt <= W) {
        bb_helper(index + 1,
                  current_weight + items[index].wt,
                  current_value + items[index].val,
                  max_value, W, items);
    }

    bb_helper(index + 1,
              current_weight,
              current_value,
              max_value, W, items);
}

int knapsack_BranchAndBound(int W, const vector<ItemSimple>& simple_items) {
    int n = simple_items.size();
    
    vector<ItemBB> items(n);
    for (int i = 0; i < n; ++i) {
        items[i].val = simple_items[i].val;
        items[i].wt = simple_items[i].wt;
        items[i].ratio = (double)simple_items[i].val / simple_items[i].wt;
    }

    sort(items.begin(), items.end(), compareItems);

    double max_value = 0.0; 
    bb_helper(0, 0, 0, max_value, W, items);
    
    return (int)max_value;
}


int main() {
    cout << fixed << setprecision(6);

    cout << "--- KICH BAN 1: Bai toan vua phai (n=20, W=100) ---" << endl;
    int W1 = 100;
    vector<ItemSimple> items1 = {
        {10, 5}, {40, 4}, {30, 6}, {50, 3}, {35, 7}, {40, 2}, {25, 8}, {15, 5},
        {55, 10}, {20, 3}, {70, 9}, {30, 6}, {60, 11}, {80, 12}, {22, 4}, {12, 2},
        {5, 1}, {90, 15}, {45, 8}, {28, 5}
    };

    auto start = high_resolution_clock::now();
    int res_bf = knapsack_BruteForce(W1, items1);
    auto stop = high_resolution_clock::now();
    auto duration_bf = duration_cast<microseconds>(stop - start);
    cout << "1. Brute Force:    Ket qua = " << res_bf 
         << ", Thoi gian = " << duration_bf.count() / 1000000.0 << " giay" << endl;

    start = high_resolution_clock::now();
    int res_bt = knapsack_Backtracking(W1, items1);
    stop = high_resolution_clock::now();
    auto duration_bt = duration_cast<microseconds>(stop - start);
    cout << "2. Backtracking:   Ket qua = " << res_bt 
         << ", Thoi gian = " << duration_bt.count() / 1000000.0 << " giay" << endl;

    start = high_resolution_clock::now();
    int res_bb = knapsack_BranchAndBound(W1, items1);
    stop = high_resolution_clock::now();
    auto duration_bb = duration_cast<microseconds>(stop - start);
    cout << "3. Branch & Bound: Ket qua = " << res_bb 
         << ", Thoi gian = " << duration_bb.count() / 1000000.0 << " giay" << endl;

    start = high_resolution_clock::now();
    int res_dp = knapsack_DP(W1, items1);
    stop = high_resolution_clock::now();
    auto duration_dp = duration_cast<microseconds>(stop - start);
    cout << "4. Dynamic Prog:   Ket qua = " << res_dp 
         << ", Thoi gian = " << duration_dp.count() / 1000000.0 << " giay" << endl;

    cout << "\n-------------------------------------------------\n" << endl;

    cout << "--- KICH BAN 2: Nhieu vat, Balo nho (n=1000, W=5000) ---" << endl;
    int W2 = 5000;
    vector<ItemSimple> items2;
    srand(1);
    for(int i = 0; i < 1000; ++i) {
        items2.push_back({rand() % 100 + 1, rand() % 50 + 1});
    }

    start = high_resolution_clock::now();
    res_dp = knapsack_DP(W2, items2);
    stop = high_resolution_clock::now();
    duration_dp = duration_cast<microseconds>(stop - start);
    cout << "4. Dynamic Prog:   Ket qua = " << res_dp 
         << ", Thoi gian = " << duration_dp.count() / 1000000.0 << " giay" << endl;

    cout << "1. Brute Force:    KHONG THE CHAY (O(n*2^n) qua lon)" << endl;
    cout << "2. Backtracking:   KHONG THE CHAY (O(2^1000) qua lon)" << endl;
    cout << "3. Branch & Bound: KHONG THE CHAY (O(2^1000) qua lon)" << endl;

    cout << "\n-------------------------------------------------\n" << endl;

    cout << "--- KICH BAN 3: It vat, Balo sieu lon (n=30, W=1 Ty) ---" << endl;
    int W3 = 1000000000;
    vector<ItemSimple> items3;
    srand(2);
    for(int i = 0; i < 30; ++i) {
        items3.push_back({rand() % 100 + 1, rand() % 50 + 1});
    }

    cout << "4. Dynamic Prog:   KHONG THE CHAY (Loi 'bad_alloc' do W qua lon)" << endl;
    cout << "1. Brute Force:    (Bo qua vi se cham hon Backtracking)" << endl;


    cout << "Dang chay Backtracking (n=30, co the mat vai giay)..." << endl;
    start = high_resolution_clock::now();
    res_bt = knapsack_Backtracking(W3, items3);
    stop = high_resolution_clock::now();
    duration_bt = duration_cast<microseconds>(stop - start);
    cout << "2. Backtracking:   Ket qua = " << res_bt 
         << ", Thoi gian = " << duration_bt.count() / 1000000.0 << " giay" << endl;
    
    
    cout << "Dang chay Branch & Bound (n=30, thuong nhanh hon)..." << endl;
    start = high_resolution_clock::now();
    res_bb = knapsack_BranchAndBound(W3, items3);
    stop = high_resolution_clock::now();
    duration_bb = duration_cast<microseconds>(stop - start);
    cout << "3. Branch & Bound: Ket qua = " << res_bb 
         << ", Thoi gian = " << duration_bb.count() / 1000000.0 << " giay" << endl;

    return 0;

}
