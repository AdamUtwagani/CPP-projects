#include <iostream>
#include <cstdlib>
#include <string>
using namespace std;


void Welcome_message();
void exit_program();   
void create_account();
void deposit();
void withdraw();
void check_balance();
void services_options();


void Welcome_message(){
	cout << "Hello Welcome to Akiba Microfinance Bank E-services" << endl;
	
}

void exit_program(){
	system("cls");
}

class Client{
	public:
		std::string balance = "1000000";
		std::string accno = "J123456789";
		
};

	
	
	void create_account(){
	string fname, lname, address, verify;
	int age;
	std::string nin, phonenumber;
	cout << "Enter the following details to create your account" << endl;
	cout << "What is your first name: " << endl;
	cin >> fname;
	cout << "What is your last name: " << endl;
	cin >> lname;
	cout << "What is your age: " << endl;
	cin >> age;
	cout << "Enter your Nida Number: " << endl;
	cin >> nin;
	cout << "What is your physical address: " << endl;
	cin >> address;
	cout << "Enter your phone number: " << endl;
	cin >> phonenumber;
	
	cout << "Here is your information entered, crosscheck for any error" << endl;
	cout << "Your name is: " << fname << " " << lname << endl;
	cout << "Your age is: " << age << endl;
	cout << "Nida number: " << nin << endl;
	cout << "Your Physical address is: " << address << endl;
	cout << "Phone Number: " << phonenumber << endl;
	cout << "Press 'y' to verify this information and create account or press 'n' to decline: " << endl;
	cin >> verify;
	
	if (verify == "y"){
		cout << "Congrats you have sucessfully created your account" << endl;
		cout << "We have sent you the details of your account through SMS" << endl;
	}else if (verify == "n") {
		system("cls");
		create_account();
	} else {
		system("cls");
		services_options();
	}
	
	
}

void deposit(){
		int deposit_amount;
		int response2;
		cout << "Enter Amount you want to deposit: " << endl;
		cin >> deposit_amount;
		cout << "Congrats! You have succesfully deposited  " << deposit_amount << endl;
		cout << "Press 1 to back to Main Menu or Press 0 to exit: " << endl;
		cin >> response2;
		if (response2 == 1){
			system("cls");
			services_options();
		} else if (response2 == 0){
			exit_program();
		} else {
			cout << "Invalid response";
		}
		
	}


void services_options(){
	int service, retry;
	cout << "Choose the services below" << endl;
	cout << "1. Create account" << endl;
	cout << "2. Deposit" << endl;
	cout << "3. Withdraw" << endl;
	cout << "4. Check Balance" << endl;
	cin >> service;
	if (service == 1){
	create_account();
	} else if (service == 2){
	deposit();
	}else if (service == 3){
	withdraw();
	} else if (service == 4){
	check_balance();
	} else {
	cout << "Invalid Choice" << endl;
	cout << "Retry enter the service you want!: " << endl;
	cin >> retry;
	services_options();
	}
	
}





void withdraw(){
	int withdraw_amount, response3;
	cout << "Enter amount you want to withdraw: " << endl;
	cin >> withdraw_amount;
	cout << "Congrats! you have succesfully withdraw " << withdraw_amount << "from your account" << endl;
	cout << "Press 1 to back to Main Menu or Press 0 to exit: " << endl;
		cin >> response3;
		if (response3 == 1){
			system("cls");
			services_options();
		} else if (response3 == 0){
			exit_program();
		} else {
			cout << "Invalid response";
		}
}

void check_balance(){
	std::string balance = "1000000";
	int response4;
	
	cout << "Your Balance is:" << balance << endl;
	cout << "Press 1 to back to Main Menu or Press 0 to exit: " << endl;
		cin >> response4;
		if (response4 == 1){
			system("cls");
			services_options();
		} else if (response4 == 0){
			exit_program();
		} else {
			cout << "Invalid response";
		}
}	







int main(){
	Welcome_message();
	services_options();
	
	
	
	
	
	
	
	
	return 0;
}