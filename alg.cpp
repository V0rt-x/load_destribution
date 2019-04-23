#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <limits>
using namespace std;

vector<int> get_int_from_string(string);
void get_data(vector<int> &, vector<int> &, vector<vector<int>> &, ifstream &);
void init_approx(vector<int> &, vector<int> &, vector<int> &);
vector<int> approx_handler(vector<int> &, vector<int> &, vector<int> &, vector<vector<int>> &);
int active_intensities_sum(vector<int> &, vector<vector<int>> &);
int active_intensities_sum_Itask(int, vector<int> &, vector<int> &);
vector<int> operation_a(vector<int> &, vector<int> &, vector<int> &, vector<vector<int>> &);
vector<int> operation_b(vector<int> &, vector<int> &, vector<int> &, vector<vector<int>> &);
vector<int> operation_c(vector<int> &, vector<int> &, vector<int> &, vector<vector<int>> &);
void print_vec(string , vector<int> &);
bool match_condition(vector<int> &, vector<int> &, vector<int> &);
void swap(int &, int &);
void swap_tasks_on_cpu(int &, vector<int> &, vector<int> &);
void debug_info(string, vector<int> &, vector<int> &, vector<int> &, vector<vector<int>> &);
void write_result_into_file(int, unsigned int, unsigned int, ofstream &, vector<vector<int>> &, int);

int main(){
    const int tries_count = 5, number_of_sets = 10;
    unsigned int start_time, end_time;
    int worst_result_num = 0;
    vector <int> proc_limits, task_loads, tasks_on_cpu, results_intensities;
    vector<vector<int>> exchg_intensities, results;
    ofstream fout("alg_output.txt", ios_base::out);
    ifstream fin("gen_output.txt", ios_base::in);
    for(int k=0; k < number_of_sets; ++k){
        worst_result_num = 0;
        proc_limits.clear();
        task_loads.clear();
        exchg_intensities.clear();
        results.clear();
        results_intensities.clear();
        tasks_on_cpu.clear();
        get_data(proc_limits, task_loads, exchg_intensities, fin);
        start_time = clock();
        for(int i=0; i < tries_count; ++i){
            init_approx(proc_limits, task_loads, tasks_on_cpu);
            results.push_back(approx_handler(proc_limits, task_loads, tasks_on_cpu, exchg_intensities));
            results_intensities.push_back(active_intensities_sum(results[i], exchg_intensities));
        }
        for(int i=1; i < results_intensities.size(); ++i)
            if(results_intensities[worst_result_num] < results_intensities[i])
                worst_result_num = i;
        end_time = clock();
        cout << k+1 << " ";
        debug_info("distribution on the CPUs", results[worst_result_num], proc_limits, task_loads, exchg_intensities);
        write_result_into_file(k, start_time, end_time, fout, exchg_intensities, results_intensities[worst_result_num]);
    }
    fout.close();
    fin.close();
    return 0;
}

vector<int> get_int_from_string(string str){
    vector<int> numbers;
    int sp_index = str.find_first_of(" ");
    string substring;
    while(sp_index != string::npos){
        sp_index = str.find_first_of(" ");
        substring = str.substr(0, sp_index);
        str.erase(0, sp_index+1);
        if(substring.length() != 0)
            numbers.push_back(atoi(substring.c_str()));
    }
    return numbers;
}

void get_data(vector<int> &proc_limits, vector<int> &task_loads, vector<vector<int>> &exchg_intensities, ifstream &fin){
    string limits_str, loads_str, intensities_str;
    int tasks_number;
    getline(fin, limits_str);
    proc_limits = get_int_from_string(limits_str);
    getline(fin, loads_str);
    task_loads = get_int_from_string(loads_str);
    tasks_number = task_loads.size();
    for(int i=0; i < tasks_number; ++i){
        getline(fin, intensities_str);
        exchg_intensities.push_back(get_int_from_string(intensities_str));
    }
}

void init_approx(vector<int> &proc_limits, vector<int> &task_loads, vector<int> &tasks_on_cpu){
    static bool srand_flag = false;
    if(!srand_flag){
        srand(time(NULL));
        srand_flag = true;
    }
    bool approx_correct = false;
    int tasks_number = task_loads.size(), proc_number = proc_limits.size();
    vector<int> employed_resource_cpu(proc_number, 0);
    while(!approx_correct){
        approx_correct = true;
        tasks_on_cpu.clear();
        employed_resource_cpu.assign(proc_number, 0);
        for(int i=0; i < tasks_number; ++i){
            tasks_on_cpu.push_back(rand() % proc_number);
            employed_resource_cpu[tasks_on_cpu[i]] += task_loads[i];
        }
        for(int i=0; i < proc_number; ++i)
            if(proc_limits[i] - employed_resource_cpu[i] < 0)                  //checking if the init approximation is correct
                approx_correct = false;
    }
}

vector<int> approx_handler(vector<int> &proc_limits, vector<int> &task_loads, vector<int> &tasks_on_cpu, \
vector<vector<int>> &exchg_intensities){
    vector<int> handling_vec, intensities;
    vector<vector<int>> results;
    int best_result_num, intensities_sum_before;
    handling_vec = tasks_on_cpu;
    do{
        best_result_num = 0;
        results.clear();
        intensities.clear();
        intensities_sum_before = active_intensities_sum(handling_vec, exchg_intensities);
        results.push_back(operation_a(proc_limits, task_loads, handling_vec, exchg_intensities));
        results.push_back(operation_b(proc_limits, task_loads, handling_vec, exchg_intensities));
        results.push_back(operation_c(proc_limits, task_loads, handling_vec, exchg_intensities));
        for(int i=0; i < results.size(); ++i)
            intensities.push_back(active_intensities_sum(results[i], exchg_intensities));
        for(int i=1; i < results.size(); ++i)
            if(intensities[best_result_num] > intensities[i])
                best_result_num = i;
        handling_vec = results[best_result_num];
    }
    while(intensities_sum_before != intensities[best_result_num]);
    return handling_vec;
}

int active_intensities_sum(vector<int> &tasks_on_cpu, vector<vector<int>> &exchg_intensities){
    int result = 0, tasks_number = tasks_on_cpu.size();
    for(int i=0; i < tasks_number; ++i)
        for(int j=i; j < tasks_number; ++j)
            if(tasks_on_cpu[i] != tasks_on_cpu[j])
                result += exchg_intensities[i][j];
    return result;
}

int active_intensities_sum_Itask(int i_task, vector<int> &tasks_on_cpu, vector<int> &exchg_intensities_Itask){
    int result = 0;
    for(int i=0; i < tasks_on_cpu.size(); ++i)
        if(tasks_on_cpu[i_task] != tasks_on_cpu[i])
            result += exchg_intensities_Itask[i];
    return result;
}

vector<int> operation_a(vector<int> &proc_limits, vector<int> &task_loads, vector<int> &tasks_on_cpu, \
vector<vector<int>> &exchg_intensities){
    int tasks_number = task_loads.size(), proc_number = proc_limits.size(), tmp_i;
    vector<int> employed_resource_cpu(proc_limits.size(), 0), new_t_o_c, prev_t_o_c;
    new_t_o_c = prev_t_o_c = tasks_on_cpu;
    for(int i=0; i < tasks_number; ++i)
        employed_resource_cpu[tasks_on_cpu[i]] += task_loads[i];
    for(int i=0; i < tasks_number; ++i)
        for(int j=0; j < proc_number; ++j)
            if(employed_resource_cpu[j] + task_loads[i] <= proc_limits[j]){
                tmp_i = new_t_o_c[i];
                new_t_o_c[i] = j;
                if(active_intensities_sum_Itask(i, prev_t_o_c, exchg_intensities[i]) < \
                active_intensities_sum_Itask(i, new_t_o_c, exchg_intensities[i]))
                    new_t_o_c[i] = tmp_i;
                else{
                    employed_resource_cpu[j] += task_loads[i];
                    prev_t_o_c = new_t_o_c;
                }
            }
    return new_t_o_c;
}

vector<int> operation_b(vector<int> &proc_limits, vector<int> &task_loads, vector<int> &tasks_on_cpu, \
vector<vector<int>> &exchg_intensities){
    int tasks_number = task_loads.size(), proc_number = proc_limits.size();
    vector<int> new_t_o_c, prev_t_o_c;
    new_t_o_c = prev_t_o_c = tasks_on_cpu;
    for(int i=0; i < tasks_number; ++i)
        for(int j=i; j < tasks_number; ++j){
            if(j != i){
                swap(new_t_o_c[i], new_t_o_c[j]);
                if((active_intensities_sum(prev_t_o_c, exchg_intensities) < active_intensities_sum(new_t_o_c, exchg_intensities)) || \
                (!match_condition(proc_limits, task_loads, new_t_o_c)))
                    swap(new_t_o_c[i], new_t_o_c[j]);
                else
                    prev_t_o_c = new_t_o_c;
            }
        }
    return new_t_o_c;
}

vector<int> operation_c(vector<int> &proc_limits, vector<int> &task_loads, vector<int> &tasks_on_cpu, \
vector<vector<int>> &exchg_intensities){
    int tasks_number = task_loads.size(), proc_number = proc_limits.size(), min_load = numeric_limits<int>::max(), swapping_load_sum;
    vector<int> new_t_o_c, prev_t_o_c, to_swap_with;
    for(int i=1; i < tasks_number; ++i)
        if(min_load > task_loads[i])
            min_load = task_loads[i];
    new_t_o_c = prev_t_o_c = tasks_on_cpu;
    for(int i=0; i < tasks_number; ++i)
        if(task_loads[i] != min_load)
            for(int j=0; j < proc_number; ++j){
                to_swap_with.clear();
                swapping_load_sum = 0;
                if(j != new_t_o_c[i]){
                    for(int k=0; k < tasks_number; ++k)
                        if((j == new_t_o_c[k]) && (task_loads[k] + swapping_load_sum <= task_loads[i]) && (i != k)){
                            to_swap_with.push_back(k);
                            swapping_load_sum += task_loads[k];
                        }
                    if((swapping_load_sum == task_loads[i]) && (to_swap_with.size() > 1)){
                        swap_tasks_on_cpu(new_t_o_c[i], to_swap_with, new_t_o_c);
                        if(active_intensities_sum(prev_t_o_c, exchg_intensities) < active_intensities_sum(new_t_o_c, exchg_intensities))
                            swap_tasks_on_cpu(new_t_o_c[i], to_swap_with, new_t_o_c);
                        else
                            prev_t_o_c = new_t_o_c;
                    }
                }
            }
    return new_t_o_c;
}

void print_vec(string vec_name, vector<int> &a){
    cout << vec_name << ": ";
    for(int i=0; i < a.size(); ++i)
        cout << a[i] << " ";
    cout << "(" << a.size() << ")" << endl;
}

bool match_condition(vector<int> &proc_limits, vector<int> &task_loads, vector<int> &tasks_on_cpu){
    int proc_number = proc_limits.size(), tasks_number = task_loads.size();
    bool correctF = true;
    vector<int> employed_resource_cpu(proc_number, 0);
    for(int i=0; i < tasks_number; ++i)
            employed_resource_cpu[tasks_on_cpu[i]] += task_loads[i];
    for(int i=0; i < proc_number; ++i)
        if(proc_limits[i] - employed_resource_cpu[i] < 0)
            correctF = false;
    return correctF;
}

void swap(int &a, int &b){
    int c = a;
    a = b;
    b = c;
}

void swap_tasks_on_cpu(int &cpu_first, vector<int> &to_swap_with, vector<int> &tasks_on_cpu){
    int tmp = cpu_first;
    cpu_first = tasks_on_cpu[to_swap_with[0]];
    for(int i=0; i < to_swap_with.size(); ++i)
        tasks_on_cpu[to_swap_with[i]] = tmp;
}

void debug_info(string vec_name, vector<int> &handling_vec, vector<int> &proc_limits, vector<int> &task_loads, \
vector<vector<int>> &exchg_intensities){    //displays summary exchange intensities and whether the vectors match the conditions on cpu limits
    print_vec(vec_name, handling_vec);
    cout << "conditions: " << match_condition(proc_limits, task_loads, handling_vec) << " exchange intensities: " << active_intensities_sum(handling_vec, exchg_intensities) << endl;
}

void write_result_into_file(int n_set, unsigned int start_time, unsigned int end_time, \
ofstream &fout, vector<vector<int>> &exchg_intensities, int result_intensity){
    int sum_intensities = 0, n = exchg_intensities.size();
    for(int i=0; i < n; ++i)
        for(int j=i; j < n; ++j)
            sum_intensities += exchg_intensities[i][j];
    fout << "Set number: " << n_set+1 << endl << "\tWorst case intensities: " << sum_intensities << endl << \
    "\tResult intensities: " << result_intensity << endl << "\tQuality of the solution: " \
    << float(result_intensity) / float(sum_intensities) << endl << "\tOperating time: " \
    << (end_time - start_time) / 1000.0 << " seconds" << endl;
}