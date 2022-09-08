/*
 * This tag combines position, velocity, and acceleration in one tag.
 */

#ifndef NACK_DATA_TAG_H
#define NACK_DATA_TAG_H

#include "ns3/tag.h"
#include "ns3/vector.h"
#include "ns3/nstime.h"

namespace ns3
{
	/** We're creating a custom tag for simulation. A tag can be added to any packet, but you cannot add a tag of the same type twice.
	*/
class NackDataTag : public Tag {
public:

	//Functions inherited from ns3::Tag that you have to implement. 
	static TypeId GetTypeId(void);
	virtual TypeId GetInstanceTypeId(void) const;
	virtual uint32_t GetSerializedSize(void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);
	virtual void Print (std::ostream & os) const;

	//These are custom accessor & mutator functions
	Vector GetPosition(void);
	uint32_t GetNodeId();
	Time GetTimestamp ();
	uint32_t GetSeqNumber();
	uint32_t GetPacketId();
	uint32_t GetNumberOfRepeat();
	int GetAmountOfBurst();
	uint8_t GetTreeNumber ();

	void SetPosition (Vector pos);
	void SetNodeId (uint32_t node_id);
	void SetTimestamp (Time t);
	void SetSeqNumber (uint32_t seq_nbr);
	void SetPacketId (uint32_t pkt_id);
	void SetNumberOfRepeat (uint32_t number_of_rpt);
	void SetAmountOfBurst (int amount_of_brst);
	void SetTreeNumber (uint8_t tree_number);


	NackDataTag();
	virtual ~NackDataTag();

	/** list of first sequence numbers of each burst */
	uint8_t *burst_first_sn;


	/** list of lengths of the burst */
	uint8_t *bursts_length;

	int put_uchar (uint8_t *bfr, int n, unsigned char x);
	int put_ulong (uint8_t *bfr, int n, unsigned long x);
	unsigned char get_uchar (uint8_t *bfr, int n);
	unsigned long get_ulong (uint8_t *bfr, int n);




private:


	Vector m_currentPosition;

	uint32_t seq_number;

	/** Timestamp this tag was created */
	Time m_timestamp;

	/** node id */
	uint32_t m_nodeId;

	/** packet id */
	uint32_t packet_id;

	/** number of retransmission */
	uint32_t number_of_repeat;

	/** amount of burst */
	int amount_of_burst;

	/** tree number of this arq line */
	uint8_t nt;











};
}

#endif 
