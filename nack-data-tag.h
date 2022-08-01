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

	void SetPosition (Vector pos);
	void SetNodeId (uint32_t node_id);
	void SetTimestamp (Time t);
	void SetSeqNumber (uint_fast32_t seq_number);



	NackDataTag();
	NackDataTag(uint32_t node_id);
	virtual ~NackDataTag();
private:

	uint32_t m_nodeId;
	/** Current position */
	Vector m_currentPosition;
	/** Timestamp this tag was created */
	Time m_timestamp;
	/** Sequence number of the packet*/
	uint32_t seq_number;

};
}

#endif 
