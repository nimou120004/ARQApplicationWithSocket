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
	burst_first_sn = new uint8_t[20];
	bursts_length = new uint8_t[20];

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
        return sizeof(Vector) + sizeof(uint32_t) + sizeof (ns3::Time) +
               sizeof(uint32_t) + sizeof(uint32_t) + sizeof (uint32_t) +
               sizeof(uint32_t) + sizeof(uint8_t) + sizeof(char[20]) +
               sizeof(char[20]);
}
/**
 * The order of how you do Serialize() should match the order of Deserialize()
 */
void NackDataTag::Serialize (TagBuffer i) const
{
	i.WriteDouble (m_currentPosition.x);
	i.WriteDouble (m_currentPosition.y);
	i.WriteDouble (m_currentPosition.z);
	i.WriteU32 (seq_number);
	i.WriteDouble(m_timestamp.GetDouble());
	i.WriteU32(m_nodeId);
	i.WriteU32 (packet_id);
	i.WriteU32 (number_of_repeat);
	i.WriteU32 (amount_of_burst);
	i.WriteU8 (nt);
	i.Write (burst_first_sn, 20);
	i.Write (bursts_length, 20);


}
/** This function reads data from a buffer and store it in class's instance variables.
 */
void NackDataTag::Deserialize (TagBuffer i)
{

	m_currentPosition.x = i.ReadDouble();
	m_currentPosition.y = i.ReadDouble();
	m_currentPosition.z = i.ReadDouble();
	seq_number = i.ReadU32 ();
	m_timestamp =  Time::FromDouble (i.ReadDouble(), Time::NS);
	m_nodeId = i.ReadU32();
	packet_id = i.ReadU32 ();
	number_of_repeat = i.ReadU32 ();
	amount_of_burst = i.ReadU32 ();
	nt = i.ReadU8 ();
	i.Read(burst_first_sn, 20);
	i.Read (bursts_length, 20);

}
/**
 * This function can be used with ASCII traces if enabled. 
 */
void NackDataTag::Print (std::ostream &os) const
{
  os << "Custom Data --- Node :" << m_nodeId <<  "\t(" << m_timestamp  << ")" << " Pos (" << m_currentPosition << ")"
     << "packet seq_number: " << seq_number;
}

int NackDataTag::put_uchar(uint8_t *bfr, int n, unsigned char x)
{
  bfr[n] = (unsigned char) x;
  return EXIT_SUCCESS;
}

int NackDataTag::put_ulong(uint8_t *bfr, int n, unsigned long x)
{
    bfr[n] = (unsigned char) (x >> 24) & 0xFF;
    bfr[n + 1] = (unsigned char) (x >> 16) & 0xFF;
    bfr[n + 2] = (unsigned char) (x >> 8) & 0xFF;
    bfr[n + 3] = (unsigned char) x & 0xFF;
    return EXIT_SUCCESS;
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

uint32_t NackDataTag::GetPacketId()
{
  return packet_id;
}

void NackDataTag::SetSeqNumber (uint32_t seq_nbr){
  seq_number = seq_nbr;
}

void NackDataTag::SetPacketId(uint32_t pkt_id)
{
    packet_id = pkt_id;
}

uint32_t NackDataTag::GetNumberOfRepeat()
{
  return number_of_repeat;
}

int NackDataTag::GetAmountOfBurst()
{
  return amount_of_burst;
}

unsigned char NackDataTag::GetTreeNumber()
{
  return nt;
}

void NackDataTag::SetNumberOfRepeat(uint32_t number_of_rpt)
{
  number_of_repeat = number_of_rpt;
}

void NackDataTag::SetAmountOfBurst(int amount_of_brst)
{
  amount_of_burst = amount_of_brst;
}

void NackDataTag::SetTreeNumber(unsigned char tree_number)
{
  nt = tree_number;
}



} /* namespace ns3 */


