#include "ServerHandlers.hpp"


uWS::Loop * p_loop{nullptr};

void assign_loop_ptr(uWS::Loop * const lp_loop)
{
    assert(lp_loop != nullptr);

    p_loop = lp_loop;
}

void connection_established_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws)
{
    ws->getUserData()->id = UniqueIdGenerator::generate_random_unique_id();

    std::cout << "Connection established with " << ws->getRemoteAddressAsText() << std::endl;
}

/*
    {
        // Every thread creates its own Loop
        // We can pass the pointer to said Loop to different threads?
        // Each thread can then call Loop::defer() (which is the only thread safe function in the library as far as I'm aware)
        //      to execute something in a safe manner
        // Here is an example of sending some data
        
        // From other threads you can call
        p_loop->defer([&ws, &msg]() {
            // Change them to smart pointers!!!
            uWS::WebSocket<false, true, PerSocketData> * p_ws = &ws; // or just "ws" depending on the types
            std::string * p_msg = msg;
            
            // IDK If creating the additional pointers is needed because IDK how memory / objects / data works with lambas when passing lambdas to be executed on different thread

            p_ws->send(*p_msg, uWS::OpCode::TEXT);
        });

        // The lambda should execute and send the data in a thread safe manner
    }
*/

void message_handler(uWS::WebSocket<false, true, PerSocketData> * ws, std::string_view msg, uWS::OpCode opCode)
{
    /* You may access ws->getUserData() here */

    std::cout << "Got message: " << msg << std::endl;

    // ws->send(msg, opCode);
    // ws->send("Does this work?", uWS::OpCode::TEXT);

    p_loop->defer(
        [&]()
        {
            ws->send("Hello from defer'ed function", uWS::OpCode::TEXT);
        }
    );
}


void connection_closed_handler(uWS::WebSocket<SSL, true, PerSocketData> * ws, int code, std::string_view msg)
{
    /* You may access ws->getUserData() here */

    std::cout << "Connection closed with " << ws->getRemoteAddressAsText() << std::endl;
}
