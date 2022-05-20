/**
 * @file schedule.cpp
 * Exam scheduling using graph coloring
 */

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <map>
#include <unordered_map>

#include "schedule.h"
#include "utils.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <climits>
using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::ifstream;

/**
 * Takes a filename and reads in all the text from the file
 * Newline characters are also just characters in ASCII
 * 
 * @param filename The name of the file that will fill the string
 */
std::string file_to_string(const std::string& filename){
  std::ifstream text(filename);

  std::stringstream strStream;
  if (text.is_open()) {
    strStream << text.rdbuf();
  }
  return strStream.str();
}

V2D file_to_V2D(const std::string & filename){
  V2D output;
  ifstream ifs(filename);
  string line;
  if (ifs.is_open()) {
    while (getline(ifs, line)) {
      vector<string> add;
      vector<string> split;
      SplitString(line, ',', split);
      for(string val : split){
        string insert = Trim(val);
        add.push_back(insert);
      }
      output.push_back(add);
    }
  }
  return output;
}

V2D clean(V2D & cv, V2D & student){
  V2D output;
  for(vector<string> course : cv){
    vector<string> insertClass;
    string currentCourse = course[0];
    insertClass.push_back(currentCourse);
    for(unsigned int i = 1; i < course.size(); i++){
      string currentStudent = course[i];
      for(vector<string> s : student){
        if(s[0] == currentStudent){
          for(unsigned int j = 1; j < s.size(); j++){
            if(s[j] == currentCourse) insertClass.push_back(currentStudent);
          }
        }
      }
    }
    output.push_back(insertClass);
  }
  for(unsigned int k = 0; k < output.size(); k++) if(output[k].size() == 1) output.erase(output.begin() + k);
  return output;
}


vector<vector<bool>> creating_a_matrix(V2D roster){
  unsigned int dim = roster.size();
  vector<vector<bool>> output;
  output.resize(dim);
  for(unsigned int i = 0; i < output.size(); i++) output[i].resize(dim, false);
  for(unsigned int i = 0; i < dim; i++){
    vector<string> currClass = roster[i];
    for(unsigned int j = 1; j < currClass.size(); j++){
      string currStudent = currClass[j];
      unsigned int count = i+1;
      while(count < dim){
        vector<string> compareClass = roster[count];
        for(unsigned int k = 1; k < compareClass.size(); k++){
          if(compareClass[k] == currStudent){
            output[i][count] = true;
            output[count][i] = true;
          }
        }
        count++;
      }
    }
  }
  return output;
}

/**
 * Given a collection of courses and a list of available times, create a valid scheduling (if possible).
 * 
 * A 'valid schedule' should assign each course to a timeslot in such a way that there are no conflicts for exams
 * In other words, two courses who share a student should not share an exam time.
 * Your solution should try to minimize the total number of timeslots but should not exceed the timeslots given.
 * 
 * The output V2D should have one row for each timeslot, even if that timeslot is not used.
 * 
 * As the problem is NP-complete, your first scheduling might not result in a valid match. Your solution should 
 * continue to attempt different schedulings until 1) a valid scheduling is found or 2) you have exhausted all possible
 * starting positions. If no match is possible, return a V2D with one row with the string '-1' as the only value. 
 * 
 * @param courses A 2D vector of strings where each row is a course ID followed by the students in the course
 * @param timeslots A vector of strings giving the total number of unique timeslots
 */
V2D schedule(V2D courses, std::vector<std::string> timeslots){
  vector<vector<bool>> roster = creating_a_matrix(courses);
  vector<vector<string>> out;
  V2D best;
  //how many classes we've given a time slot
  int numClassesReached = 0;
  //the total number of classes we have to give a time slot
  int numClasses = roster.size();
  //the class we're going to start with to give a time slot
  int startPoint = 0;
  //the index of the class we're giving a time slot
  int index = 0; 
  //make an out vector that is the size of how many time slots there are
  out.resize(timeslots.size());
  //loop through each start point
  while(startPoint < numClasses){
    //loop until you've reached every class
    while(numClassesReached != numClasses){
      //if we're at the start point they get the first time slot
      if(index == startPoint){
        out[0].push_back(courses[index][0]);
      }
      else{
        //get which classes the current classes shares students with
        vector<string> connections = getConnections(roster, index, courses);
        //variable for the timeslot index we're at
        unsigned int thullu = 0;
        //go through each row in the output 
        for(vector<string> classes : out){
          //variable to check if the class needs to be added to a new time slot or not 
          bool canAdd = true;
          //go through all fo the classes connections and see if any of them are present in the timeslot
          for(string c : connections){
            //if there is a connection present set canAdd to false
            if (std::find(classes.begin(), classes.end(), c) != classes.end()){
              canAdd = false;
              break;
            }
          }
          //add it to the timeslot index if there were no connections present (canAdd is true)
          if(canAdd){
            out[thullu].push_back(courses[index][0]);
            break;
          }
          //if you couldn't add go to the next time slot
          thullu += 1;
        }
      }
      //go to the next class (increment index)
      index += 1;
      //if we're at the end of the list go back to the start and continue from there
      if(index == numClasses) index = 0;
      //increment the number of classes we've added
      numClassesReached += 1;
    }
    //check to see if schedule is valid 
    int sum = 0;
    for(unsigned int i = 0; i < out.size(); i++){
      sum += out[i].size();
    }
    //if it is return it
    if(sum == numClasses){
      return out;
    }
    //else reset all variables to create a new possible schedule
    numClassesReached = 0;
    startPoint += 1;
    index = startPoint;
    for(vector<string> vect: out) vect.clear();
  }
  //if best was not returned already no valid schedule was made
  best.resize(1, {"-1"});
  return best;
}

vector<string> getConnections(vector<vector<bool>> roster, int index, V2D courses){
  vector<string> output;
  vector<bool> wanted = roster[index]; 
  for(unsigned int i = 0; i < wanted.size(); i++){
    if(wanted[i]) output.push_back(courses[i][0]);
  }
  return output;
}
