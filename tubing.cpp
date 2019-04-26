#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#define MAX_TUBE_NUM 255
#define EPS .01f
using namespace std;
int n_tubes = 0; // Number of different tubes available
int n_segments;  // Number of segments needed
float min_diam, max_diam;
float min_tolerance, max_tolerance;

ofstream fout("tubing.txt");
class tube
{
  public:
    float outer_diam;
    float thickness;
    tube(float outer_diam, float thickness)
    {
        this->outer_diam = outer_diam;
        this->thickness = thickness;
    }
    float get_inner_diam()
    {
        return outer_diam - 2 * thickness;
    }
};
bool tube_cmp(const tube &a, const tube &b)
{
    if ((a.outer_diam < b.outer_diam) || (a.outer_diam == b.outer_diam && a.thickness < b.thickness))
    {
        return true;
    }
    return false;
}
vector<tube> list_tubes;
int* result_tmp;
vector<int*> results;
// DP data structures
struct node
{
    vector<int> next_nodes;
    int n_segments = 0; // number of segments formed till now
};
struct node dp[MAX_TUBE_NUM];
// trace back
void print_tube(int idx, int n)
{
    if(n < 1) return;
    result_tmp[n_segments - n] = idx; 
    if(n == 1)
    {
        int *result_now = new int[n_segments];
        memcpy(result_now, result_tmp, n_segments * sizeof(int));
        results.push_back(result_now);
        return;
    }
    else if(n > 1)
    {
        for(int i = 0; i < dp[idx].next_nodes.size(); ++i)
        {
            print_tube(dp[idx].next_nodes[i], n - 1);
        }
    }
}
int main(int argc, char **argv)
{
    ifstream f_param("param.txt");
    ifstream f_size("size.txt");
    string s;
    // read task parameters
    // structure: n_segments:int min_diam:float max_diam:float min_tolerance:float max_tolerance:float
    // skip comments
    while (getline(f_param, s))
    {
        if (s[0] != '#')
            continue;
        f_param >> n_segments >> min_diam >> max_diam >> min_tolerance >> max_tolerance;
        break;
    }
    printf("%d %.1f %.1f %.1f %.1f\n", n_segments, min_diam, max_diam, min_tolerance, max_tolerance);
    f_param.close();
    // read tubing size sheet
    // each line: outer diam(mm)xthickness(mm)
    float outer_diam, thickness;
    while (getline(f_size, s))
    {
        if (s[0] == '#')
            continue;
        sscanf(s.c_str(), "%fx%f", &outer_diam, &thickness);
        list_tubes.push_back(tube(outer_diam, thickness));
        n_tubes += 1;
    }
    // sort tubes by ascending robustness (first outer_diam then thickness)
    sort(list_tubes.begin(), list_tubes.end(), tube_cmp);

    printf("%d tube sizes available\n", n_tubes);
    // core algorithm
    // DP
    int idx_min_diam = 0;
    int idx_max_diam = n_tubes;
    while (list_tubes[idx_min_diam].outer_diam < min_diam - EPS)
        ++idx_min_diam;
    for (int i = idx_min_diam; i < n_tubes; ++i)
    {
        struct tube tube_now = list_tubes[i];
        if(tube_now.outer_diam > max_diam + EPS) 
        {
            idx_max_diam = i;
            break;
        }
        dp[i].n_segments = 1;
        int max_n_segments = 0;
        for(int j=i-1; j>=idx_min_diam; --j)
        {
            struct tube tube_target = list_tubes[j];
            float space = tube_now.get_inner_diam() - tube_target.outer_diam;
            if(space > min_tolerance - EPS && space < max_tolerance + EPS) // if they match   
            {
                if(dp[j].n_segments > max_n_segments) max_n_segments = dp[j].n_segments;
                dp[i].next_nodes.push_back(j);
            }
            dp[i].n_segments = max_n_segments + 1;
        }
    }
    int n_sols = 0;
    // output results
    result_tmp = new int[n_segments];
    for (int i = idx_min_diam; i < idx_max_diam; ++i)
    {
        if(dp[i].n_segments >= n_segments)
        {
            print_tube(i, n_segments);
        }
    }
    // print results
    auto iter = results.begin();
    while(iter != results.end())
    {
        int* r = *iter;
        for(int i=0; i<n_segments; ++i)
        {
            tube tube_now = list_tubes[r[i]];
            // printf("%.1fx%.1f--", tube_now.outer_diam, tube_now.thickness);
            fout << tube_now.outer_diam << 'x' << tube_now.thickness << "--";
        }
        // printf("\n");
        fout << endl;
        ++iter;
    }
    fout.close();
    printf("%d solutions found\n", results.size());
    return 0;
}