# Flower exchange Trader application

- The flower exchange is a system which
supports basic trading
-  Exchange Application - will process the incoming order against existing orders in the 
order container(known as Order Book) and do a full or partial execution.
- Every order is replied with an Execution Report by the Exchange Application indicating the status of the order

# Architecture

Even though we could use a high level architecture where trader application and exchange application work independently and communicate via sockets, we decided to user a much more simpler architecture in this project

![Architecture](images/architecture.png)

