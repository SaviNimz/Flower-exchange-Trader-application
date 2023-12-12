# Flower exchange Trader application

- The flower exchange is a system which
supports basic trading
-  Exchange Application - will process the incoming order against existing orders in the 
order container(known as Order Book) and do a full or partial execution.
- Every order is replied with an Execution Report by the Exchange Application indicating the status of the order

## Table of Contents

- [Architecture](#Architecture)
- [Getting Started](#getting-started)
- [Input and output](#input-and-output)
- [Implementation Details](#Implementation-details)


# Architecture

Even though we could use a high level architecture where trader application and exchange application work independently and communicate via sockets, we decided to user a much more simpler architecture in this project

![Architecture](images/architecture.png)


## Getting Started

1. Clone the repository:

   ```bash
   git clone https://github.com/SaviNimz/Flower-exchange-Trader-application.git

2. Run the executable file of application.cpp
    ```bash
    .\application.exe 

## Input and output

In the absence of a graphical user interface (GUI), input parameters will be specified through a Comma-Separated Values (CSV) file. The outcome of the execution process will be systematically documented and recorded in a CSV file

- input file

![input](images/orders%20csv.png)

- output file

![output](images/execution%20rep%20csv.png)


## Implementation details 


- We've used an object oriented approach in the implementation

There are few structures and one main class in the main code. [Order] (application.cpp#L17-L37) is  one of the most important of them




