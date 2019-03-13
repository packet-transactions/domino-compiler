#include <vector>
#include <iostream>
#include <algorithm>

//Output the group in format like [1,2][3,4]
void print_group(std::vector<int> arr,int group_size){
  //Pay attention to the case where the group_size is 1 
  for (int i=0;i!=arr.size();i++){
    if(i%group_size == 0)
      std::cout<<"["<<arr[i]<<",";
    else if (i%group_size == group_size -1)
      std::cout<<arr[i]<<"]";
    else
      std::cout<<arr[i]<<",";
  }
  std::cout<<std::endl;
}

//Generate all permutations
void Permutations(std::vector<int> origin_list,std::vector<std::vector<int> > &permutation){
  while(next_permutation(origin_list.begin(),origin_list.end())){
    permutation.push_back(origin_list);
  }
}

//Sort all permutations
void sort_list(std::vector<std::vector<int> >& permutation_list,int group_size){
  //Sort each group member by ascending order
  for (int i=0;i!=permutation_list.size();i++)
    for (int j=0;j<permutation_list[i].size();j=j+group_size)
      sort(permutation_list[i].begin()+j,permutation_list[i].begin()+j+group_size);
  return;
  //Sort the number of each group
  for (int i=0;i!=permutation_list.size();i++)
    for (int j=0;j<permutation_list[i].size()-group_size;j=j+group_size)
      if (permutation_list[i][j] > permutation_list[i][j+group_size])
        for (int k=j;k!=j+group_size;k++){
          int tmp = permutation_list[i][k];
          permutation_list[i][k] = permutation_list[i][k+group_size];
          permutation_list[i][k+group_size] = tmp;
        }
}

//Whether two vectors are the same
int is_same(std::vector<int> a,std::vector<int> b){
  for (int i=0;i!=a.size();i++){
    if (a[i]!=b[i])
      return 0;
  }
  return 1;
}

//Whether this vector has appeared in the vector<vector>
int is_in_vector(std::vector<int> arr, std::vector<std::vector<int> >&permutation_without_repeat){
  for (int i=0;i!=permutation_without_repeat.size();i++){
    if (is_same(arr,permutation_without_repeat[i]))
      return 1;
  }
  return 0;
}

//Delete the repeated member in the list
void del_repeat(std::vector<std::vector<int> >permutation_list,std::vector<std::vector<int> >&permutation_without_repeat){
  for (int i=0;i!=permutation_list.size();i++){
    if (i==0)
      permutation_without_repeat.push_back(permutation_list[i]);
    else{
      if (!is_in_vector(permutation_list[i],permutation_without_repeat))
        permutation_without_repeat.push_back(permutation_list[i]);
    }
  }
}

//generate all combinations
void combination_generator(std::vector<int> arr,std::vector<int> data,int start,int end,int index, int r,
                     std::vector< std::vector<int> >& res){
  if(index == r){
    res.push_back(data);
    return;
  }
  int i = start;
  while(i <= end && end - i + 1 >= r - index){
    data[index] = arr[i];
    combination_generator(arr, data, i + 1, end, index + 1, r , res);
    i++;
  }
}

int main(){
  std::vector<int> arr;
  //group_size means the maximum group size
  int group_size = 1;
  //total_number means how many state vars
  int total_number = 4;
  //Fill in arr
  for (int i=1;i!=total_number+1;i++){
    arr.push_back(i);
  }
  //Pay attention to the case where the group_size is 1
  if (group_size == 1){
    for (int j=0;j!=arr.size();j++){
        if (j!= arr.size()-1)
          std::cout<<"["<< arr[j] << "]";
        else
          std::cout<<"["<< arr[j] << "]"<<std::endl;
      }
    return 0;
  }
  for (int i = 0;i<=arr.size()/group_size;i++){
    if (i==0){
      for (int j=0;j!=arr.size();j++){
        if (j!= arr.size()-1)
          std::cout<<"["<< arr[j] << "]";
        else
          std::cout<<"["<< arr[j] << "]"<<std::endl;
      }
      continue;
    }
    std::vector< std::vector<int> > res;
    std::vector<int> data;
    //num_to_pick means how many state vars to pick
    int num_to_pick = group_size * i;//this is a for loop
    for (int j =0;j!=num_to_pick;j++){
      data.push_back(0);
    }
    //generate combination
    combination_generator(arr,data,0,arr.size()-1,0,num_to_pick,res);
    for (int p=0;p!=res.size();p++){
      std::vector<std::vector<int> > permutation;
      std::vector<std::vector<int> > permutation_without_repeat;
      //generate permutation
      Permutations(res[p],permutation);
      //generate sort_list
      sort_list(permutation,group_size);
      //delete the repeated member
      del_repeat(permutation,permutation_without_repeat);
      //Print out result
      for (int k=0;k!=permutation_without_repeat.size();k++){
        //Print the member in Group of size 1
        for (int index=0;index!=arr.size();index++)
          //if the member of arr have not appear in permutation_without_repeat[k]
          if(find(permutation_without_repeat[k].begin(),permutation_without_repeat[k].end(),arr[index]) == permutation_without_repeat[k].end())
           std::cout <<"[" << arr[index] << "]";
        print_group(permutation_without_repeat[k],group_size);
      }
    }
  }
  return 0;
}
