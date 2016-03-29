/*
	First attempt to implement SLBN in the data plane
	using DOMINO.

	Opeartions that are currently not supported:
		****** Division ******
		int temp1 = (LINK_CAP - BF)/(NR + 1);
		int temp2 = LINK_CAP/N;
		
		****** array operations on packet field ******
		pkt.B.exists(LINK_ID)
		pkt.B.remove(LINK_ID)

	SLBN Paper: http://ieeexplore.ieee.org/xpls/abs_all.jsp?arnumber=6299097&tag=1
*/

#define MAX_HOPS 	100
#define LINK_CAP 	100 
#define LINK_ID		1

// Declare the state stored on the switch per link
int BF = 0; // total bandwidth allocated to saturated flows
int NR = 0; // total number of unsaturated flows
int N = 0; // total number of flows that cross this link

struct Packet {
	int type;
		/*
			ControlPacket Types:
			0 ==> Join
			1 ==> Probe
			2 ==> ProbeAck
			3 ==> Leave
		*/
	int s; 				// session to which the packet belongs
	int bwpp; 			// bandwidth computed 2 probe cycles ago
	int bwp; 			// bandwidth computed in previous probe cycle
	int bw; 			// bandwidth currently being computed
		// *** DOMINO does not allow arrays in packet fields -- need a workaround for this
	int b; 				// latest bottleneck that was added to B

	int numBNecks; // indicates the number of elements in B

        int temp1;
        int temp2;
        int shBW;
};

void RouterLink(struct Packet pkt) {

	if (pkt.type == 0) {
		N = N + 1;
		
		// int shBW = max( (LINK_CAP - BF)/(NR + 1), LINK_CAP/N ); 
		pkt.temp1 = (LINK_CAP - BF)/(NR + 1);
		pkt.temp2 = LINK_CAP/N;
		pkt.shBW = (pkt.temp1 > pkt.temp2) ? pkt.temp1 : pkt.temp2;

		if (pkt.bw >= pkt.shBW) {
			pkt.b = LINK_ID;
			NR = NR + 1;
		}
		// send downstream Join(s,bwpp,bwp,bw,B,b)
	}
	else if (pkt.type == 1 ) {
		{
			BF = BF - pkt.bwpp;
			NR = NR + 1;
		}
		// int shBW = max( (LINK_CAP - BF)/(NR + 1), LINK_CAP/N ); 
		pkt.temp1 = (LINK_CAP - BF)/(NR + 1);
		pkt.temp2 = LINK_CAP/N;
		pkt.shBW = (pkt.temp1 > pkt.temp2) ? pkt.temp1 : pkt.temp2;

		if ( (LINK_ID == pkt.b) || (pkt.bw >= pkt.shBW)) {
			pkt.bw = pkt.shBW;
			pkt.b = LINK_ID;
		}
		else { // pkt.bw < shBW
			BF = BF + pkt.bwp;
			NR = NR - 1;
		}
		// send downstream Probe(s,bwpp,bwp,bw,B,b)
	}
	else if (pkt.type == 2) {
		{
			BF = BF - pkt.bwp;
			NR = NR + 1;
		}
		// int shBW = max( (LINK_CAP - BF)/(NR + 1), LINK_CAP/N ); 
		pkt.temp1 = (LINK_CAP - BF)/(NR + 1);
		pkt.temp2 = LINK_CAP/N;
		pkt.shBW = (pkt.temp1 > pkt.temp2) ? pkt.temp1 : pkt.temp2;

		if ( (LINK_ID == pkt.b) || (pkt.bw >= pkt.shBW) ) {
			pkt.bw = pkt.shBW;
			pkt.b = LINK_ID;
		}
		else { // pkt.bw < shBW
			BF = BF + pkt.bwp;
			NR = NR - 1;
		}
		// send upstream ProbeAck(s,bwpp,bwp,bw,B,b)
	}
	else if (pkt.type == 3) {
                {
			BF = BF - pkt.bwp;
		}
		N = N - 1;
		// send downstream(s,bwpp,bwp,bw,B,b)
	}
}
