#include "../src/Globals.cpp"

#include <string>
#include <rclcpp/rclcpp.hpp>

// Local dependencies
#include "WebSocketServer.h"

using namespace Globals;

class MissionControlInterface : public rclcpp::Node {
  public:
    MissionControlInterface() : Node("MissionControlInterface_node"), 
                                server_("DefaultServer", WS_SERVER_PORT) {
      
    }
  private:
    net::websocket::SingleClientWSServer server_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  rclcpp::spin(std::make_shared<MissionControlInterface>());
  rclcpp::shutdown();

  return 0;
}
