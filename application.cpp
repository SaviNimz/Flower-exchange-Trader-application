#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <chrono>
#include <ctime>

#include <queue> // priority queue header

using namespace std;

ifstream input_file("Orders.csv");         // Replace "input.csv" with the input CSV file's name
ofstream output_file("Execution_Rep.csv"); // Replace "output.csv" with the output CSV file's name

struct Order
{
    string clientOrderID;
    string orderID;
    string instrument;
    int side;
    int status;
    int quantity;
    double price;
    string reason;

    Order(string orderID, string clientOrderID, string instrument, int side, int status, int quantity, double price)
    {
        this->orderID = orderID;
        this->clientOrderID = clientOrderID;
        this->instrument = instrument;
        this->side = side;
        this->status = status;
        this->quantity = quantity;
        this->price = price;
    }

    string toString()
    {
        stringstream ss;
        ss << orderID << "," << clientOrderID << "," << instrument << "," << side << "," << status << "," << quantity << "," << price << "," << reason;
        return ss.str();
    }
};

struct orderbook_Entry{

    string OrderID;
    string ClietOrderID;
    int Quantity;
    int price;

    orderbook_Entry(string OrderID, string ClietOrderID,int Quantity, int price){
        this -> OrderID = OrderID;
        this -> ClietOrderID = ClietOrderID;
        this -> Quantity = Quantity;
        this -> price = price;
    }
    // Define the comparison operators
    bool operator>(const orderbook_Entry& other) const {
        return price > other.price;
    }

    bool operator<(const orderbook_Entry& other) const {
        return price < other.price;
    }
};
// Custom comparator to compare orderbook_Entry objects based on their prices
struct CompareEntry {
    bool operator()(const orderbook_Entry& a, const orderbook_Entry& b) {
        return a.price < b.price;
    }
};


class OrderBook
{

public:
    string instrument;
    vector<Order> orders;

    // Max heap for buy orders
    priority_queue<orderbook_Entry, vector<orderbook_Entry>, CompareEntry> buy_orders;
    // Min heap for sell orders
    priority_queue<orderbook_Entry, vector<orderbook_Entry>, greater<orderbook_Entry>> sell_orders;


    OrderBook(const string& instrument)
        : instrument(instrument) {}

    void addOrder(Order& order)

    {
        // first do all the validations for the order to check if it si valid 

        if (order.price < 0)
        {
            rejectOrder(order, "Invalid price");
            return;
        }

        if (order.quantity % 10 != 0 || order.quantity < 10 || order.quantity > 1000)
        {
            rejectOrder(order, "Invalid size");
            return;
        }

        if (order.side != 1 && order.side != 2)
        {
            rejectOrder(order, "Invalid side");
            return;
        }

        // if non of the above conditions are met we can go to the execution report generation
        // Add order to the appropriate heap based on the side

        // create an object of the order book entry

        string temp_OrderID = order.orderID;
        string temp_clientID = order.clientOrderID;
        int temp_quantity = order.quantity;
        int temp_price = order.price;

        orderbook_Entry temp_entry = orderbook_Entry(temp_OrderID,temp_clientID,temp_quantity,temp_price);

        if (order.side == 1) // Buy order
        {
            // need to check through the sell  side for a matching order
            // if the order is matching it's selling price should be less than or equal to buying price
            // then update the amounts and write them to the execution report
            // if we still  have matching sell orders for the remaining amount do the process again
            processBuyOrder(temp_entry,order);
        }
        else if (order.side == 2) // Sell order
        {
            processSellOrder(temp_entry,order);
        }
    }

    void writeTo_Execution_report(ofstream& output_file, Order& order, const vector<string>& reportStrings)
    {
        // Write all accumulated strings to the output file
        for (const auto& str : reportStrings) {
            output_file << str << endl;
        }
    }
private:
    vector<string> executionReportStrings;
    void rejectOrder(const Order& order, const string& reason)
    {
        Order rejectedOrder = order;
        rejectedOrder.reason = reason;
        rejectedOrder.status = 1; // Rejected
        output_file << rejectedOrder.toString() << endl;
    }

    void processBuyOrder(orderbook_Entry& buyOrder,Order& order) {

        // this boolean value is used to check if the order is new 
        bool isModified = false;
        while (!sell_orders.empty() && sell_orders.top().price <= buyOrder.price) {
            // Execute the order with the lowest sell price
            orderbook_Entry sellOrder = sell_orders.top();
            sell_orders.pop();

            // Update the quantities, generate execution report, etc.
            buyOrder.Quantity = buyOrder.Quantity - sellOrder.Quantity;

            order.quantity = sellOrder.Quantity;
            order.price = sellOrder.price;
            
            isModified = true;

            Order ExecRep_sell_ord = Order(sellOrder.OrderID,sellOrder.ClietOrderID,order.instrument,2,2,sellOrder.Quantity,sellOrder.price);

            // Check if there exists remaining quantity in the buy order
            if (buyOrder.Quantity == 0) {
                order.status = 2;
                // Accumulate strings to the vector
                executionReportStrings.push_back(order.toString());
                executionReportStrings.push_back(ExecRep_sell_ord.toString());
                break; // The buy order is completely filled
            }
            else{
                // partially filled order
                order.status = 3;
            // Accumulate strings to the vector
                executionReportStrings.push_back(order.toString());
                executionReportStrings.push_back(ExecRep_sell_ord.toString());
            }
        }

        if (buyOrder.Quantity > 0 && !isModified) {
            // Add the remaining buy order to the buy_orders heap
            buy_orders.push(buyOrder);
            executionReportStrings.push_back(order.toString());
            // write the order to the execution report as a new order
        }
        else{
            buy_orders.push(buyOrder);
        }
        // Call writeTo_Execution_report once
        writeTo_Execution_report(output_file, order, executionReportStrings);
        // Clear the vector for the next use
        executionReportStrings.clear();
    }

    void processSellOrder(orderbook_Entry& sellOrder, Order& order) {
        // This boolean value is used to check if the order is new
        bool isModified = false;

        while (!buy_orders.empty() && buy_orders.top().price >= sellOrder.price) {
            // Execute the order with the highest buy price
            orderbook_Entry buyOrder = buy_orders.top();
            buy_orders.pop();

            // Update the quantities, generate execution report, etc.
            sellOrder.Quantity = sellOrder.Quantity - buyOrder.Quantity;

            order.quantity = buyOrder.Quantity;
            order.price = buyOrder.price;

            isModified = true;

            Order ExecRep_buy_ord = Order(buyOrder.OrderID, buyOrder.ClietOrderID, order.instrument, 1, 2, buyOrder.Quantity, buyOrder.price);

            // Check if there exists remaining quantity in the sell order
            if (sellOrder.Quantity == 0) {
                order.status = 2;
                executionReportStrings.push_back(order.toString());
                executionReportStrings.push_back(ExecRep_buy_ord.toString());
                break; // The sell order is completely filled
            } else {
                // Partially filled order
                order.status = 3;
                executionReportStrings.push_back(order.toString());
                executionReportStrings.push_back(ExecRep_buy_ord.toString());
            }
        }
        if (sellOrder.Quantity > 0 && !isModified) {
            // Add the remaining sell order to the sell_orders heap
            sell_orders.push(sellOrder);
            executionReportStrings.push_back(order.toString());
            // Write the order to the execution report as a new order
        }else{
            // add the order to the order book
            sell_orders.push(sellOrder);
        }
                
        // Call writeTo_Execution_report once
        writeTo_Execution_report(output_file, order, executionReportStrings);
        // Clear the vector for the next use
        executionReportStrings.clear();
    }


};


int main()

{
    if (!input_file.is_open())
    {
        cerr << "Could not open the input file." << endl;
        return 1;
    }

    if (!output_file.is_open())
    {
        cerr << "Could not create or open the output file." << endl;
        return 1;
    }
    // Create a map to associate instrument names with order books
    map<string, OrderBook> orderBooks;

    orderBooks.emplace("Rose", "Rose");
    orderBooks.emplace("Lavender", "Lavender");
    orderBooks.emplace("Lotus", "Lotus");
    orderBooks.emplace("Tulip", "Tulip");
    orderBooks.emplace("Orchid", "Orchid");

    // Skip the header rows
    string line;
    getline(input_file, line); // Skip the header row
    getline(input_file, line); // Skip the field names row

    // Add the header row to the output file
    output_file << "execution_rep.csv" << endl;

    // Add the field names row to the output file
    output_file << "Order ID, Client Order ID, Instrument, Side, Execution Status, Quantity, Price, Reason" << endl;

    int order_id = 1;

    while (getline(input_file, line))
    {
        istringstream iss(line);
        vector<string> row;
        string field;

        while (getline(iss, field, ','))
        {
            row.push_back(field);
        }

        string instrument = row[1];
        auto it = orderBooks.find(instrument);

        // check weather if we can find the correct instrument in the hashmap of orderbooks 

        if (it != orderBooks.end())
        {

            // Create an order
            Order order("ord" + to_string(order_id), row[0], instrument, stoi(row[2]), 0, stoi(row[3]), stod(row[4]));

            // Add the order to the appropriate order book
            it->second.addOrder(order);

            order_id++; // Increment order ID for each valid order
        }
        else
        {
            // instrument validation will happen here
            // Invalid instrument
            Order order("ord" + to_string(order_id), row[0], instrument, stoi(row[2]), 1, stoi(row[3]), stod(row[4]));
            order.reason = "Invalid instrument";
            output_file << order.toString() << endl;
        }
    }

    return 0;
}


