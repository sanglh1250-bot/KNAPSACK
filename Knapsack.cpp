#include <iostream>
#include <vector>       
#include <algorithm>    
#include <chrono>      
#include <numeric>     
#include <iomanip>      


using namespace std;
using namespace std::chrono;

/**
 * @brief Cấu trúc lưu trữ vật đơn giản, dùng cho 3/4 thuật toán.
 */
struct ItemSimple {
    int val; // Giá trị (value)
    int wt;  // Trọng lượng (weight)
};

/**
 * @brief Cấu trúc vật dụng cho Branch & Bound, cần thêm tỉ lệ v/w.
 */
struct ItemBB {
    int val;
    int wt;
    double ratio; // Tỉ lệ val / wt
};

/**
 * @brief Hàm so sánh cho std::sort, sắp xếp theo tỉ lệ giảm dần.
 */
bool compareItems(ItemBB a, ItemBB b) {
    return a.ratio > b.ratio;
}


//--- 1. THUẬT TOÁN: BRUTE FORCE (VÉT CẠN) ---
// Độ phức tạp: O(n * 2^n)

int knapsack_BruteForce(int W, const vector<ItemSimple>& items) {
    int n = items.size();
    int max_value = 0;
    
    // 1LL << n là 2^n (sử dụng long long để tránh tràn số với n > 31)
    long long num_subsets = 1LL << n;

    // Duyệt qua tất cả 2^n tập con bằng kỹ thuật bitmask
    for (long long i = 0; i < num_subsets; ++i) {
        int current_weight = 0;
        int current_value = 0;

        for (int j = 0; j < n; ++j) {
            // Kiểm tra bit thứ j có được bật (giá trị 1) hay không
            // (i >> j) & 1 có nghĩa là "vật j có trong tập con i không?"
            if ((i >> j) & 1) {
                current_weight += items[j].wt;
                current_value += items[j].val;
            }
        }

        // Nếu tập con hợp lệ (không quá cân) và giá trị tốt hơn
        if (current_weight <= W) {
            max_value = max(max_value, current_value);
        }
    }
    return max_value;
}


//--- 2. THUẬT TOÁN: BACKTRACKING (QUAY LUI) ---
// Độ phức tạp: O(2^n)

/**
 * @brief Hàm đệ quy (helper) cho thuật toán Backtracking.
 * @param index Chỉ số của vật đang xét.
 * @param current_weight Tổng trọng lượng hiện tại.
 * @param current_value Tổng giá trị hiện tại.
 * @param max_value Tham chiếu đến giá trị tốt nhất tìm được.
 */
void backtrack_helper(int index, int current_weight, int current_value,
                      int& max_value, int W, const vector<ItemSimple>& items) {
    int n = items.size();

    // Điều kiện dừng (base case): đã xét hết tất cả các vật
    if (index == n) {
        max_value = max(max_value, current_value);
        return;
    }

    // Nhánh 1: KHÔNG chọn vật thứ 'index'
    // Đi tiếp đến vật (index + 1) mà không thay đổi trọng lượng hay giá trị
    backtrack_helper(index + 1, current_weight, current_value, max_value, W, items);

    // Nhánh 2: Thử CHỌN vật thứ 'index'
    // Chỉ đi vào nhánh này nếu việc thêm vật không vượt quá W (đây là cắt tỉa)
    if (current_weight + items[index].wt <= W) {
        backtrack_helper(index + 1,
                         current_weight + items[index].wt,
                         current_value + items[index].val,
                         max_value, W, items);
    }
}

int knapsack_Backtracking(int W, const vector<ItemSimple>& items) {
    int max_value = 0;
    // Bắt đầu từ vật 0, trọng lượng 0, giá trị 0
    backtrack_helper(0, 0, 0, max_value, W, items);
    return max_value;
}


//--- 3. THUẬT TOÁN: DYNAMIC PROGRAMMING (QUY HOẠCH ĐỘNG) ---
// Độ phức tạp: O(n * W)

int knapsack_DP(int W, const vector<ItemSimple>& items) {
    int n = items.size();
    
    // Tối ưu bộ nhớ: chỉ dùng 1 mảng 1 chiều O(W)
    // dp[j] = giá trị lớn nhất có thể đạt được với trọng lượng j
    vector<int> dp(W + 1, 0);

    for (int i = 0; i < n; ++i) {
        // Duyệt ngược từ W xuống wt[i]
        // để đảm bảo mỗi vật chỉ được chọn 1 lần (0/1 Knapsack)
        for (int j = W; j >= items[i].wt; --j) {
            // dp[j] (không chọn vật i)
            // dp[j - items[i].wt] + items[i].val (chọn vật i)
            dp[j] = max(dp[j], dp[j - items[i].wt] + items[i].val);
        }
    }
    
    // Kết quả cuối cùng nằm ở dp[W]
    return dp[W];
}


//--- 4. THUẬT TOÁN: BRANCH AND BOUND (NHÁNH VÀ CẬN) ---
// Độ phức tạp: O(2^n) (Worst case)

/**
 * @brief Tính cận trên (upper bound) cho một nút.
 * Đây là ước tính lạc quan (greedy) bằng cách dùng Knapsack phân số.
 */
double calculate_bound(int index, int current_weight, int current_value,
                       int W, const vector<ItemBB>& items) {
    int n = items.size();
    int w = current_weight;
    double v = current_value;

    // Tính toán lạc quan (Greedy - Knapsack phân số)
    for (int i = index; i < n; ++i) {
        if (w + items[i].wt <= W) {
            // Lấy trọn vật i
            w += items[i].wt;
            v += items[i].val;
        } else {
            // Lấy một phần của vật i để lấp đầy balo
            double remaining_w = W - w;
            v += items[i].ratio * remaining_w;
            break; // Balo đã đầy
        }
    }
    return v;
}

/**
 * @brief Hàm đệ quy (helper) cho thuật toán Branch and Bound.
 * @param max_value Dùng kiểu double để so sánh với 'bound' cho chính xác.
 */
void bb_helper(int index, int current_weight, int current_value,
               double& max_value, int W, const vector<ItemBB>& items) {
    int n = items.size();

    // Cập nhật kết quả tốt nhất (cận dưới)
    // Chỉ cập nhật từ các lời giải hoàn chỉnh (giá trị nguyên)
    if (current_weight <= W && current_value > max_value) {
        max_value = (double)current_value;
    }

    // Điều kiện dừng: đã xét hết vật
    if (index == n) {
        return;
    }

    // Tính cận trên (upper bound)
    double bound = calculate_bound(index, current_weight, current_value, W, items);

    // Cắt tỉa (Pruning) quan trọng:
    // Nếu cận trên (ước tính lạc quan nhất) còn tệ hơn kết quả đã có (max_value)
    // thì không cần đi tiếp nhánh này.
    if (bound <= max_value) {
        return;
    }

    // Nhánh 1: Thử CHỌN vật 'index' (nếu có thể)
    if (current_weight + items[index].wt <= W) {
        bb_helper(index + 1,
                  current_weight + items[index].wt,
                  current_value + items[index].val,
                  max_value, W, items);
    }

    // Nhánh 2: KHÔNG chọn vật 'index'
    bb_helper(index + 1,
              current_weight,
              current_value,
              max_value, W, items);
}

int knapsack_BranchAndBound(int W, const vector<ItemSimple>& simple_items) {
    int n = simple_items.size();
    
    // 1. Chuẩn bị dữ liệu cho B&B
    vector<ItemBB> items(n);
    for (int i = 0; i < n; ++i) {
        items[i].val = simple_items[i].val;
        items[i].wt = simple_items[i].wt;
        items[i].ratio = (double)simple_items[i].val / simple_items[i].wt;
    }

    // 2. Sắp xếp các vật theo tỉ lệ v/w giảm dần (quan trọng)
    sort(items.begin(), items.end(), compareItems);

    double max_value = 0.0; // Cận dưới ban đầu
    bb_helper(0, 0, 0, max_value, W, items);
    
    // Trả về kết quả (ép kiểu về int)
    return (int)max_value;
}


//--- HÀM MAIN ĐỂ CHẠY THỬ VÀ ĐÁNH GIÁ ---

int main() {
    // Đặt định dạng in số thập phân
    cout << fixed << setprecision(6);

    // ---- KỊCH BẢN 1: Bài toán NHỎ (n=20) ----
    // n=20 -> 2^20 ~ 1 triệu. Tất cả thuật toán đều chạy được.
    // W=100.
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


    // ---- KỊCH BẢN 2: "BÀI TOÁN KHÓ" (DP thắng) ----
    // n = 1000 (rất lớn), W = 5000 (nhỏ)
    // O(2^n) = 2^1000 -> Không thể chạy
    // O(n*W) = 1000 * 5000 = 5,000,000 -> Rất nhanh
    cout << "--- KICH BAN 2: Nhieu vat, Balo nho (n=1000, W=5000) ---" << endl;
    int W2 = 5000;
    vector<ItemSimple> items2;
    srand(1); // Khởi tạo seed để kết quả ổn định
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


    // ---- KỊCH BẢN 3: "BÀI TOÁN KHÓ" (B&B / Backtracking thắng) ----
    // n = 30 (nhỏ), W = 1,000,000,000 (siêu lớn)
    // O(2^n) = 2^30 ~ 1 tỷ -> Chậm, nhưng chạy được
    // O(n*W) = 30 * 1 tỷ = 30 tỷ -> Rất chậm VÀ KHÔNG THỂ CẤP PHÁT BỘ NHỚ
    cout << "--- KICH BAN 3: It vat, Balo sieu lon (n=30, W=1 Ty) ---" << endl;
    int W3 = 1000000000;
    vector<ItemSimple> items3;
    srand(2);
    for(int i = 0; i < 30; ++i) {
        items3.push_back({rand() % 100 + 1, rand() % 50 + 1});
    }

    cout << "4. Dynamic Prog:   KHONG THE CHAY (Loi 'bad_alloc' do W qua lon)" << endl;
    // Bỏ qua Brute Force vì nó chậm hơn Backtracking một cách không cần thiết
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