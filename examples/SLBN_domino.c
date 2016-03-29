/*
	First attempt to implement SLBN in the data plane
	using DOMINO.

	Opeartions that are currently not supported:
		****** Division ******
		int temp1 = (LINK_CAP - BF)/(NR + 1);
		int temp2 = LINK_CAP/N;
		
		****** array operations on packet field ******
		pkt.B.append(LINK_ID)
		pkt.B.exists(LINK_ID)
		pkt.B.remove(LINK_ID)

	SLBN Paper: http://ieeexplore.ieee.org/xpls/abs_all.jsp?arnumber=6299097&tag=1
*/

#define MAX_HOPS 	100
#define LINK_CAP 	100 
#define LINK_ID		1

struct ControlPacket {
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
	int B[MAX_HOPS]; 	// set of bottlenecks for this session
		// *** DOMINO does not allow arrays in packet fields -- need a workaround for this
	int b; 				// latest bottleneck that was added to B

	int numBNecks; // indicates the number of elements in B

        int temp1;
        int temp2;
        int shBW;
}

// Declare the state stored on the switch per link
int BF = 0; // total bandwidth allocated to saturated flows
int NR = 0; // total number of unsaturated flows
int N = 0; // total number of flows that cross this link

void RouterLink(struct ControlPacket pkt) {

	if (pkt.type == 0) {
		N = N + 1;
		
		// int shBW = max( (LINK_CAP - BF)/(NR + 1), LINK_CAP/N ); 
		pkt.temp1 = (LINK_CAP - BF)/(NR + 1);
		pkt.temp2 = LINK_CAP/N;
		pkt.shBW = (temp1 > temp2) ? temp1 : temp2;

		if (pkt.bw >= shBW) {
			pkt.B.append(LINK_ID);	
			pkt.b = LINK_ID;
			NR = NR + 1;
		}
		// send downstream Join(s,bwpp,bwp,bw,B,b)
	}
	else if (pkt.type == 1 ) {
		if (pkt.B.exists(LINK_ID)) { 	
			pkt.B.remove(LINK_ID);		
		}
		else {
			BF = BF - pkt.bwpp;
			NR = NR + 1;
		}
		// int shBW = max( (LINK_CAP - BF)/(NR + 1), LINK_CAP/N ); 
		pkt.temp1 = (LINK_CAP - BF)/(NR + 1);
		pkt.temp2 = LINK_CAP/N;
		pkt.shBW = (temp1 > temp2) ? temp1 : temp2;

		if ( (LINK_ID == pkt.b) || (pkt.bw >= shBW)) {
			pkt.B = pkt.B.append(LINK_ID);		
			pkt.bw = shBW;
			pkt.b = LINK_ID;
		}
		else { // pkt.bw < shBW
			BF = BF + pkt.bwp;
			NR = NR - 1;
		}
		// send downstream Probe(s,bwpp,bwp,bw,B,b)
	}
	else if (pkt.type == 2) {
		if (pkt.B.exists(LINK_ID)) {
			pkt.B.remove(LINK_ID);
		}
		else {
			BF = BF - pkt.bwp;
			NR = NR + 1;
		}
		// int shBW = max( (LINK_CAP - BF)/(NR + 1), LINK_CAP/N ); 
		pkt.temp1 = (LINK_CAP - BF)/(NR + 1);
		pkt.temp2 = LINK_CAP/N;
		pkt.shBW = (temp1 > temp2) ? temp1 : temp2;

		if ( (LINK_ID == pkt.b) || (pkt.bw >= shBW) ) {
			pkt.B.append(LINK_ID);
			pkt.bw = shBW;
			pkt.b = LINK_ID;
		}
		else { // pkt.bw < shBW
			BF = BF + pkt.bwp;
			NR = NR - 1;
		}
		// send upstream ProbeAck(s,bwpp,bwp,bw,B,b)
	}
	else if (pkt.type == 3) {
		if (pkt.B.exists(LINK_ID)) {
			NR = NR - 1;
		}
		else {
			BF = BF - pkt.bwp;
		}
		N = N - 1;
		// send downstream(s,bwpp,bwp,bw,B,b)
	}
} 


void append(int link_id) {
	if ( pkt.numBNecks < MAX_HOPS) {
		pkt.B[pkt.numBNecks] = link_id;
		pkt.numBNecks++;
	}
}

void exists(int link_id) {
	// Require hardware implementation of parallel search on an array (pkt.B)
	// This is a required primative to run at line rate
	// Amount of required hardware is proportional to MAX_HOPS (the size of the B field in the packet)
}

void remove(int link_id) {
	// Require hardware implementation of parallel search and removal of element from array
	pkt.numBNecks--;
}
