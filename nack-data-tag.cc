/*
 * cacc-data-tag.cc
 *
 *  Created on: Oct 29, 2019
 *      Author: adil
 */
#include "nack-data-tag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("NackDataTag");
NS_OBJECT_ENSURE_REGISTERED (NackDataTag);

NackDataTag::NackDataTag() {
	m_timestamp = Simulator::Now();
	m_nodeId = -1;
}
NackDataTag::NackDataTag(uint32_t node_id) {
	m_timestamp = Simulator::Now();
	m_nodeId = node_id;
}


NackDataTag::~NackDataTag() {
}

//Almost all custom tags will have similar implementation of GetTypeId and GetInstanceTypeId
TypeId NackDataTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NackDataTag")
    .SetParent<Tag> ()
    .AddConstructor<NackDataTag> ();
  return tid;
}
TypeId NackDataTag::GetInstanceTypeId (void) const
{
  return NackDataTag::GetTypeId ();
}

/** The size required for the data contained within tag is:
 *   	size needed for a ns3::Vector for position +
 * 		size needed for a ns3::Time for timestamp + 
 * 		size needed for a uint32_t for node id
 */
uint32_t NackDataTag::GetSerializedSize (void) const
{
        return sizeof(Vector) + sizeof (ns3::Time) + sizeof(uint32_t) + sizeof(uint32_t);
}
/**
 * The order of how you do Serialize() should match the order of Deserialize()
 */
void NackDataTag::Serialize (TagBuffer i) const
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
void NackDataTag::Deserialize (TagBuffer i)
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
void NackDataTag::Print (std::ostream &os) const
{
  os << "Custom Data --- Node :" << m_nodeId <<  "\t(" << m_timestamp  << ")" << " Pos (" << m_currentPosition << ")"
     << "packet seq_number: " << seq_number;
}

//Your accessor and mutator functions 
uint32_t NackDataTag::GetNodeId() {
	return m_nodeId;
}

void NackDataTag::SetNodeId(uint32_t node_id) {
	m_nodeId = node_id;
}
Vector NackDataTag::GetPosition(void) {
	return m_currentPosition;
}

void NackDataTag::SetPosition(Vector pos) {
	m_currentPosition = pos;
}

Time NackDataTag::GetTimestamp() {
	return m_timestamp;
}

void NackDataTag::SetTimestamp(Time t) {
	m_timestamp = t;
}

uint32_t NackDataTag::GetSeqNumber (){
	return seq_number;
}

void NackDataTag::SetSeqNumber (uint_fast32_t seq_nbr){
	seq_number = seq_nbr;
}



} /* namespace ns3 */


