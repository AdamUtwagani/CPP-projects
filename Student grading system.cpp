#include <iostream>
#include <cstdlib>
using namespace std;

void gradingsystem(){
	string fname,lname,course, grade;
		int level, math, history, ics, physics, biology, total, average;
		
		cout << "Enter student first name: ";
		cin >> fname;
		
		cout << "Enter student last name: ";
		cin >> lname;
		
		cout << "Enter course: ";
		cin >> course;
		
		cout << "Now enter marks of the following subjects" << endl;
		cout << "Mathematics: ";
		cin >> math;
		
		cout << "History: ";
		cin >> history;
		
		cout << "ICS: ";
		cin >> ics;
		
		cout << "Physics: ";
		cin >> physics;
		
		cout << "Biology: ";
		cin >> biology;
		
		total = math + history + ics + physics + biology;
		average = total / 5;
		
		if (average >= 80){
			grade = "A";
		} else if (average >= 65 and average <= 79){
			grade = "B";
		} else if (average >= 45 and average <= 64){
			grade = "C";
		}else {
			grade = "Failed";
		}
		
		cout << "Total Marks is: " << total << endl;
		cout << "With an Average of: " << average << endl;
		cout << "Hence Student's Grade is: " << grade << endl;
}

int main(){
	string proceed;
	while(true) {
		system("cls");
		gradingsystem();
		cout << "Do you want to continue with another Student? 'y' for yes and 'n' to exit: " <<endl;
		cin >> proceed;
		if (proceed == "n"){
			cout << "Bye";
			break;
		} else if ( proceed != "y"){
			cout << "Invalid choice";
			break;
		}
	}
	
	
	return 0;
}



