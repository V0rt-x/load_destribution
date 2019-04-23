#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <ctime>
using namespace std;

void input_data(int &, int[3], int[4], int &, int[4]);
void read_from_file(int &, int[3], int[4], int &, int[4]);
void read_from_cin(int &, int[3], int[4], int &, int[4]);
int generator(int, int[3] , int[4], int, int[4]);
int select_by_prob(int *, int, bool *restr = 0);

ofstream fout("gen_output.txt", ios_base::out);

int main(){
    const int number_of_generations = 10;
    int num_of_proc, task_probabilities[3], processor_probabilities[4], Q, intensity_probabilities[4];

    input_data(num_of_proc, task_probabilities, processor_probabilities, Q, intensity_probabilities);
    for(int i=0; i < number_of_generations; ++i)
        generator(num_of_proc, task_probabilities, processor_probabilities, Q, intensity_probabilities);

    fout.close();
    return 0;
}

void input_data(int &num_of_proc, int prob_task[3] , int prob_proc[4], int &Q, int prob_intensity[4]){
    int respond;
    cout << "Do you want to read from 'gen_input.txt'? (1 - yes/ 0 - no)" << endl;
    cin >> respond;
    switch(respond){
        case 1: read_from_file(num_of_proc, prob_task, prob_proc, Q, prob_intensity); break;
        case 0: read_from_cin(num_of_proc, prob_task, prob_proc, Q, prob_intensity);
    }    
}

void read_from_cin(int &num_of_proc, int prob_task[3] , int prob_proc[4], int &Q, int prob_intensity[4]){
    cout << "Number of CPUs:" << endl;
    cin >> num_of_proc;
    cout << "Probabilities of matching CPU loads with tasks (3):" << endl;
    for(int i = 0; i < 3; ++i)
        cin >> prob_task[i];
    cout << "Probabilities of matching CPUs with their limits (4):" << endl;
    for(int i = 0; i < 4; ++i)
        cin >> prob_proc[i];
    cout << "Percentage of total performance resouce:" << endl;
    cin >> Q;
    cout << "Probabilities of matching exchanges between tasks intensities with pairs of tasks (4):" << endl;
    for(int i = 0; i < 4; ++i)
        cin >> prob_intensity[i];
}

void read_from_file(int &num_of_proc, int prob_task[3] , int prob_proc[4], int &Q, int prob_intensity[4]){
    ifstream fin("gen_input.txt", ios_base::in);
    fin >> num_of_proc;
    for(int i = 0; i < 3; ++i)
        fin >> prob_task[i];
    for(int i = 0; i < 4; ++i)
        fin >> prob_proc[i];
    fin >> Q;
    for(int i = 0; i < 4; ++i)
        fin >> prob_intensity[i];
}

int generator(int num_of_proc, int prob_task[3] , int prob_proc[4], int Q, int prob_intensity[4]){
    int ctrl1 = 0, ctrl2 = 0;
    for(int i=0; i<3; ++i){
        if(prob_task[i] < 0)
            return -1;
        ctrl1 += prob_task[i];
    }
    if(ctrl1 != 100)
        return -1;
    ctrl1 = 0;
    for(int i=0; i < 4; ++i){
        if((prob_proc[i] < 0) || (prob_intensity[i] < 0))
             return -1;
        ctrl1 += prob_proc[i];
        ctrl2 += prob_intensity[i];
    }
    if((ctrl1 != 100) || (ctrl2 != 100))
        return -1;

    int probable_proc_limits[4] = {40, 60, 80, 100};
    int probable_task_loads[3] = {5, 10, 20};
    int probable_exchange_intensities[4] = {0, 10, 50, 100};

    int proc_limits[num_of_proc], resource=0;                                       
    for(int i=0; i < num_of_proc; ++i){
        proc_limits[i] = probable_proc_limits[select_by_prob(prob_proc, 4)];        //p.1
        resource += proc_limits[i];                                                 //p.2
    }
    resource = float(resource) * float(Q) / 100;                                    //p.3
    bool restrictions[3] = {false, false, false};                                   //false - possible to create the task; true - impossible
    vector<int> task_loads;
    int new_task_load, new_task_index;
    while(!restrictions[0] || !restrictions[1] || !restrictions[2]){                //p.6
        new_task_index = select_by_prob(prob_task, 3, restrictions);
        new_task_load = probable_task_loads[new_task_index];
        if(resource - new_task_load < 0)
            restrictions[new_task_index] = true;                                    //p.5 (restrict overloading tasks)
        else{
            task_loads.push_back(new_task_load);                                    //p.4
            resource -= new_task_load;                                              //(p.7) recalculation of free resource
        }
    }
    int tasks_count = task_loads.size();
    int intensities[tasks_count][tasks_count], new_intensity_index;
    for(int i = 0; i < tasks_count; ++i)
        for(int j = i; j < tasks_count; ++j){
            if(i != j){
                new_intensity_index = select_by_prob(prob_intensity, 4);
                intensities[j][i] = intensities[i][j] = probable_exchange_intensities[new_intensity_index];
            }
            else
                intensities[i][j] = 0;
        }

    for(int i=0; i < num_of_proc; ++i)
        fout << proc_limits[i] << " ";
    fout << endl;
    for(int i=0; i < tasks_count; ++i)
        fout << task_loads[i] << " ";
    fout << endl;
    for(int i=0; i < tasks_count; ++i){
        for(int j=0; j < tasks_count; ++j)
            fout << intensities[i][j] << " ";
        fout << endl;
    }

    return 0;
}

int select_by_prob(int *probs, int k, bool *restr){   //returns index of selected item
    bool default_restr = false;
    if(restr == 0){
        restr = (bool *)malloc(size_t(k));
        for(int i=0; i < k; ++i)
            restr[i] = false;
        default_restr = true;
    }
    if(k < 1)
        return -1;
    int check_sum = 0;
    for(int i=0; i < k; ++i){
        check_sum += int(!restr[i]);
    }
    if(check_sum == 0)
        return -2;                                                                  //if all loads are restricted
    static bool srand_flag = false;
    if(!srand_flag){
        srand(time(NULL));
        srand_flag = true;
    }
    int result;
    do{
        int r = rand() % 100 + 1;
        int sum = 0;
        for(int i=0; i < k; ++i){
            sum += probs[i];
            if(r <= sum){
                result = i;
                break;
            }
        }
    }
    while(restr[result]);
    if(default_restr)
        free(restr);
    return result;
}
