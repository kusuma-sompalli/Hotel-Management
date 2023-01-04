#include<vector>
using namespace std;

class Hotel{
public:
        void customer_login(){
                string firstname;
                string lastname;
                string email;
                string address;
                string number;
                cout << "Enter Your Firstname: ";
                getline(cin,firstname);
                cout << "Enter your lastame: ";
                getline(cin,lastname);
                cout << "Enter your Email: ";
                getline(cin,email);
                cout << "Enter Your Address: ";
                getline(cin,address);
                cout << "Enter Your number: ";
                getline(cin,number);
                vector<string> single_customer;
                vector<vector<string> > all_customers;
                single_customer.push_back(firstname);
                single_customer.push_back(lastname);
                single_customer.push_back(email);
                single_customer.push_back(address);
                single_customer.push_back(address);
                single_customer.push_back(number);
                all_customers.push_back(single_customer);
                for (int i=0;i < all_customers.size();i++){
                        for (int j=0;j<single_customer.size();j++){
                                cout << all_customers[i][j] << " ";
                        }
                        cout <<endl;
                }

        }


};

int main(){
        Hotel H1;
        Hotel H2;
        H1.customer_login();
        H2.customer_login();

}
