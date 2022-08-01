/*
 * cacc-data-tag.cc
 *
 *  Created on: Oct 29, 2019
 *      Author: adil
 */
#include "packet-data-tag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("PacketDataTag");
NS_OBJECT_ENSURE_REGISTERED (PacketDataTag);

PacketDataTag::PacketDataTag() {
	m_timestamp = Simulator::Now();
	m_nodeId = -1;
}
PacketDataTag::PacketDataTag(uint32_t node_id) {
	m_timestamp = Simulator::Now();
	m_nodeId = node_id;
}


PacketDataTag::~PacketDataTag() {
}

//Almost all custom tags will have similar implementation of GetTypeId and GetInstanceTypeId
TypeId PacketDataTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PacketDataTag")
    .SetParent<Tag> ()
    .AddConstructor<PacketDataTag> ();
  return tid;
}
TypeId PacketDataTag::GetInstanceTypeId (void) const
{
  return PacketDataTag::GetTypeId ();
}

/** The size required for the data contained within tag is:
 *   	size needed for a ns3::Vector for position +
 * 		size needed for a ns3::Time for timestamp + 
 * 		size needed for a uint32_t for node id
 */
uint32_t PacketDataTag::GetSerializedSize (void) const
{
        return sizeof(Vector) + sizeof (ns3::Time) + sizeof(uint32_t) + sizeof(uint32_t);
}
/**
 * The order of how you do Serialize() should match the order of Deserialize()
 */
void PacketDataTag::Serialize (TagBuffer i) const
{
	//we store timestamp first
	i.WriteDouble(m_timestamp.GetDouble());

	//then we store the position
	i.WriteDouble (m_currentPosition.x);
	i.WriteDouble (m_currentPosition.y);
	i.WriteDouble (m_currentPosition.z);

	//Then we store the node ID
	i.WriteU32(m_nodeId);

	//Then packet sequence number
	i.WriteU32 (seq_number);
}
/** This function reads data from a buffer and store it in class's instance variables.
 */
void PacketDataTag::Deserialize (TagBuffer i)
{
	//We extract what we stored first, so we extract the timestamp
	m_timestamp =  Time::FromDouble (i.ReadDouble(), Time::NS);;

	//Then the position
	m_currentPosition.x = i.ReadDouble();
	m_currentPosition.y = i.ReadDouble();
	m_currentPosition.z = i.ReadDouble();
	//Then, we extract the node id
	m_nodeId = i.ReadU32();
	//Then we extract the packet sequnce number
	seq_number = i.ReadU32 ();

}
/**
 * This function can be used with ASCII traces if enabled. 
 */
void PacketDataTag::Print (std::ostream &os) const
{
  os << "Custom Data --- Node :" << m_nodeId <<  "\t(" << m_timestamp  << ")" << " Pos (" << m_currentPosition << ")"
     << "packet seq_number: " << seq_number;
}

//Your accessor and mutator functions 
uint32_t PacketDataTag::GetNodeId() {
	return m_nodeId;
}

void PacketDataTag::SetNodeId(uint32_t node_id) {
	m_nodeId = node_id;
}
Vector PacketDataTag::GetPosition(void) {
	return m_currentPosition;
}

void PacketDataTag::SetPosition(Vector pos) {
	m_currentPosition = pos;
}

Time PacketDataTag::GetTimestamp() {
	return m_timestamp;
}

void PacketDataTag::SetTimestamp(Time t) {
	m_timestamp = t;
}

uint32_t PacketDataTag::GetSeqNumber (){
	return seq_number;
}

void PacketDataTag::SetSeqNumber (uint32_t seq_nbr){
	seq_number = seq_nbr;
}

Address PacketDataTag::GetSenderAddr (){
        return sender_addr;
}
void PacketDataTag::SetSenderAddr (Address sender_address){
        sender_addr = sender_address;
}



} /* namespace ns3 */


