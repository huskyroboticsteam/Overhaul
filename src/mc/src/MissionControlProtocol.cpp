#include "../include/mc/MissionControlProtocol.h"

#include <mc/msg/mc_packet.hpp>
#include <rclcpp/rclcpp.hpp>

namespace mc {

MissionControlProtocol::MissionControlProtocol() : Node("mc_node") {

}


} // namespace mc

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);

  rclcpp::executors::SingleThreadedExecutor executor;
  rclcpp::Node::SharedPtr node = std::make_shared<mc::MissionControlProtocol>();
  executor.add_node(node);

	std::thread thread = std::thread([&]() { executor.spin(); });

	while (rclcpp::ok()) {}

  rclcpp::shutdown();
  thread.join();

  return 0;
}
