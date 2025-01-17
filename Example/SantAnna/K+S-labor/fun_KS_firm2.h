/******************************************************************************

	FIRM2 OBJECT EQUATIONS
	----------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the Firm2 objects in the K+S LSD model
	are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "_Bon2" )
/*
Bonuses to pay by firm in consumption-good sector
*/
v[1] = V( "_Pi2" ) - V( "_Tax2" );				// firm net profit in period
v[2] = VL( "_K", 1 );							// available capital in period
RESULT( v[1] > 0 && v[2] > 0 && v[1] / v[2] > VLS( PARENT, "Pi2rateAvg", 1 ) &&
		V( "_L2" ) > 0 ? VS( LABSUPL2, "psi6" ) * v[1] : 0 )


EQUATION( "_Deb2max" )
/*
Prudential maximum bank debt of firm in consumer-good sector
Also updates '_CD2', '_CD2c', '_CS2'
*/

// maximum debt allowed to firm, considering net worth and operating margin
v[1] = VS( FINSECL2, "Lambda" ) * max( VL( "_NW2", 1 ),
									   VL( "_S2", 1 ) - VL( "_W2", 1 ) );

// apply an absolute floor to maximum debt prudential limit
v[0] = max( v[1], VS( FINSECL2, "Lambda0" ) * VLS( CAPSECL2, "PPI", 1 ) /
				  VS( CAPSECL2, "pK0" ) );

WRITE( "_CD2", 0 );								// reset firm credit demand
WRITE( "_CD2c", 0 );							// reset credit constraint
WRITE( "_CS2", 0 );								// reset total credit supplied

RESULT( v[0] )


EQUATION( "_Div2" )
/*
Dividends to pay by firm in consumption-good sector
*/
RESULT( max( VS( PARENT, "d2" ) * ( V( "_Pi2" ) - V( "_Tax2" ) - V( "_Bon2" ) ),
			 0 ) )


EQUATION( "_D2e" )
/*
Adaptive demand expectation of firm in consumer-good sector
*/

if ( V( "_life2cycle" ) < 3 )					// entrant?
	// myopic-optimistic expectations
	END_EQUATION( max( VL( "_D2d", 1 ), CURRENT ) );

k = VS( GRANDPARENT, "flagExpect" );			// expectation form
j = ( k == 0 || k > 4 ) ? 1 : ( k == 1 ) ? 4 : 2;// req. number of data periods

// compute the mix between fulfilled and potential demand (orders)
v[9] = VS( PARENT, "e0" );						// animal spirits parameter
for ( i = 1; i <= j; ++i )
{
	v[10] = VL( "_D2", i );
	v[ i ] = max( ( 1 - v[9] ) * v[10] + v[9] * VL( "_D2d", i ), v[10] );
}

switch ( k )
{
	// myopic expectations with 1-period memory
	case 0:
	default:
		v[0] = v[1];
		break;

	// myopic expectations with up to 4-period memory
	case 1:
		v[11] = VS( PARENT, "e1" );				// weight of t-1 demand
		v[12] = VS( PARENT, "e2" );				// weight of t-2 demand
		v[13] = VS( PARENT, "e3" );				// weight of t-3 demand
		v[14] = VS( PARENT, "e4" );				// weight of t-4 demand

		for ( v[5] = v[6] = 0, i = 1; i <= 4; ++i )
			if ( v[i] > 0 )						// consider only periods with demand
			{
				v[5] += v[ 10 + i ] * v[ i ];
				v[6] += v[ 10 + i ];
			}

		v[0] = v[6] > 0 ? v[5] / v[6] : 0;		// rescale
		break;

	// accelerating GD expectations
	case 2:
		v[2] = max( v[2], 1 );					// floor to positive only

		v[0] = ( 1 + VS( PARENT, "e5" ) * ( v[1] - v[2] ) / v[2] ) * v[1];
		break;

	// 1st order adaptive expectations
	case 3:
		v[0] = CURRENT + VS( PARENT, "e6" ) * ( v[1] - v[2] );
		break;

	// extrapolative-accelerating expectations
	case 4:
		v[2] = max( v[2], 1 );					// floor to positive only

		v[0] = ( 1 + VS( PARENT, "e7" ) * ( v[1] - v[2] ) / v[2] +
				 VS( PARENT, "e8" ) * VLS( GRANDPARENT, "dGDP", 1 ) ) * v[1];
		break;
}

RESULT( v[0] )


EQUATION( "_E" )
/*
Effective competitiveness of a firm in sector 2, considering the price,
unfilled demand and the quality of the product for the consumer
*/

if ( V( "_life2cycle" ) == 0 )
	END_EQUATION( VLS( PARENT, "Eavg", 1 ) );	// non-producing entrant

v[1] = VS( PARENT, "p2min" );					// market parameters
v[2] = VS( PARENT, "p2max" );
v[3] = VLS( PARENT, "l2min", 1 );
v[4] = VLS( PARENT, "l2max", 1 );
v[5] = VS( PARENT, "q2min" );
v[6] = VS( PARENT, "q2max" );
v[7] = VS( PARENT, "omega1" );					// competitiveness weights
v[8] = VS( PARENT, "omega2" );
v[9] = VS( PARENT, "omega3" );

// normalize price, unfilled demand, and quality to [0.1, 0.9]
// zero competitiveness is avoided to prevent direct exit of worst in replicator
v[10] = v[2] > v[1] ? 0.1 + 0.8 * ( V( "_p2" ) - v[1] ) / ( v[2] - v[1] ) : 0.5;
v[11] = v[4] > v[3] ? 0.1 + 0.8 * ( VL( "_l2", 1 ) - v[3] ) / ( v[4] - v[3] ) : 0.5;
v[12] = v[6] > v[5] ? 0.1 + 0.8 * ( V( "_q2" ) - v[5] ) / ( v[6] - v[5] ) : 0.5;

RESULT( v[7] * ( 1 - v[10] ) + v[8] * ( 1 - v[11] ) + v[9] * v[12] )


EQUATION( "_EI" )
/*
Effective expansion investment of firm in consumption-good sector
Also updates '_NW2', '_Deb2', _CD2', '_CD2c', '_CS2'
*/
V( "_Q2" );										// make sure production decided
V( "_supplier" );								// ensure supplier is selected
RESULT( invest( THIS, V( "_EId" ) ) )


EQUATION( "_EId" )
/*
Desired expansion investment of firm in consumption-good sector
*/

v[1] = V( "_Kd" );								// desired capital
v[2] = VL( "_K", 1 );							// available capital stock
v[3] = VS( PARENT, "m2" );						// machine output per period

if ( v[2] < v[3] )								// no capital yet?
	END_EQUATION( v[1] );						// no growth threshold

v[4] = VS( PARENT, "kappaMin" );				// investment floor multiple

// min rounded capital
v[5] = round( ( 1 + v[4] ) * v[2] / v[3] ) * v[3];

if ( v[1] > v[5] )								// minimum capital reached?
{
	if ( v[4] > 0 )
		v[0] = v[5] - v[2];
	else
	{
		v[6] = VS( PARENT, "kappaMax" );		// investment cap multiple
		v[7] = round( ( 1 + v[6] ) * v[2] / v[3] ) * v[3];

		if ( v[6] > 0 && v[1] > v[7] )
			v[0] = v[7] - v[2];					// max rounded capital
		else
			v[0] = floor( ( v[1] - v[2] ) / v[3] ) * v[3];
	}
}
else
	v[0] = 0;									// no expansion investment

RESULT( v[0] )


EQUATION( "_Kd" )
/*
Desired capital stock of firm in consumption-good sector
*/

if ( V( "_life2cycle" ) == 0 )					// if fresh entrant
	END_EQUATION( CURRENT );					// keep initially desired capital

// desired capacity with slack and utilization, based on expectations/inventories
RESULT( max( ( 1 + VS( PARENT, "iota" ) ) * V( "_D2e" ) - VL( "_N", 1 ), 0 ) /
		VS( PARENT, "u" ) )


EQUATION( "_Q2" )
/*
Planned production for a firm in consumption-good sector
Also updates '_NW2', '_NW2p', '_Deb2', '_CD2', '_CD2c', '_CS2'
*/

v[1] = V( "_Q2d" );								// desired production
v[2] = V( "_CS2a" );							// available credit supply
v[3] = VL( "_NW2", 1 );							// net worth (cash available)
v[4] = V( "_c2" );								// expected unit cost
v[5] = VS( PARENT, "m2" );						// machine output per period

v[6] = v[1] * v[4];								// cost of desired production

if ( v[6] <= v[3] )								// firm can self-finance?
{
	v[0] = v[1];								// plan the desired output
	v[7] = v[3] - v[6];							// remove wage cost from cash
	v[8] = 0;									// no finance
}
else
{
	if ( v[6] <= v[3] + v[2] )					// possible to finance all?
	{
		v[0] = v[1];							// plan the desired output
		v[7] = 0;								// no cash
		v[8] = v[9] = v[6] - v[3];				// finance the difference
	}
	else										// credit constrained firm
	{
		// produce as much as the available finance allows, rounded # machines
		v[0] = floor( max( ( v[3] + v[2] ) / v[4], 0 ) / v[5] ) * v[5];
		v[9] = v[6] - v[3];						// desired credit

		if ( v[0] == 0 )
		{
			v[7] = v[3];						// keep cash
			v[8] = 0;							// no finance
		}
		else
		{
			v[6] = v[0] * v[4];					// reduced production cost
			if ( v[6] <= v[3] )
			{
				v[7] = v[3] - v[6];				// pay with available cash
				v[8] = 0;						// no finance
			}
			else
			{
				v[7] = 0;						// no cash
				v[8] = v[6] - v[3];				// finance the difference
			}
		}
	}

	update_debt( THIS, v[9], v[8] );			// update debt (desired/granted)
}

update_depo( THIS, v[7], false );				// update the firm net worth
WRITE( "_NW2p", v[3] - v[7] + v[8] );			// provision for production

RESULT( v[0] )


EQUATION( "_Q2d" )
/*
Desired output of firm in consumption-good sector
*/

if ( V( "_life2cycle" ) == 0 )					// if fresh entrant
	END_EQUATION( 0 );							// not yet producing

// desired production with slack, based on expectations, considering inventories
v[1] = max( ( 1 + VS( PARENT, "iota" ) ) * V( "_D2e" ) - VL( "_N", 1 ), 0 );

// limited to the available capital stock
RESULT( min( v[1], VL( "_K", 1 ) ) )


EQUATION( "_SI" )
/*
Effective substitution investment of firm in consumption-good sector
Also updates '_NW2', '_Deb2', _CD2', '_CD2c', '_CS2'
*/
V( "_EI" );										// make sure expansion done
RESULT( invest( THIS, V( "_SId" ) ) )


EQUATION( "_Tax2" )
/*
Tax paid by firm in consumption-good sector
Also updates '_NW2', '_Deb2', _CD2', '_CD2c', '_CS2'
*/

v[1] = V( "_Pi2" );								// firm profit in period

if ( v[1] > 0 )									// profits?
	v[0] = v[1] * VS( GRANDPARENT, "tr" );		// tax to government
else
	v[0] = 0;									// no tax on losses

cash_flow( THIS, v[1], v[0] );					// manage the period cash flow

RESULT( v[0] )


EQUATION( "_alloc2" )
/*
Allocate workers to vintages, prioritizing newer vintages
*/

cur = HOOK( TOPVINT );							// start with top vintage
if ( cur == NULL )
	END_EQUATION( 0 );							// no vintage, nothing to do

V( "_L2" );										// ensure hiring is done
V( "_c2" );										// ensure machines are allocated
SUM( "__dLdVint" );								// unallocate unneeded workers

h = VS( GRANDPARENT, "flagWorkerLBU" );			// worker-level learning mode
bool vint_learn = ( h != 0 && h != 2 );			// learning-by-vintage in use?

// first, allocate unallocated workers to top vintages
// going back from newest vintage to oldest, if needed
i = 0;											// allocations counter
k = VS( cur, "__dLdVint" );						// addt'l labor demand of vint.
CYCLE( cur1, "Wrk2" )							// search for unallocated worker
	if ( HOOKS( SHOOKS( cur1 ), VWRK ) == NULL )
	{
		while ( k == 0 )						// find vintage with open posit.
		{
			cur = SHOOKS( cur );				// so go to the previous one
			if ( cur == NULL || VS( cur, "__toUseVint" ) == 0 )// oldest or done?
				goto done_alloc2;				// not possible to allocate more

			k = VS( cur, "__dLdVint" );			// addt'l labor demand of vint.
		}

		move_worker( SHOOKS( cur1 ), cur, vint_learn );// move worker to vintage
		++i;
		--k;									// update vintage worker demand
	}

// then, if needed, try to move workers from older to newer vintages
// keep going back while not get to oldest or the last vintage in use
CYCLE( cur1, "Vint" )							// search for unallocated worker
{
	j = COUNTS( cur1, "WrkV" );
	while ( j > 0 )
	{
		while ( k == 0 )						// find vintage with open posit.
		{
			cur = SHOOKS( cur );				// so go to the previous one
			if ( cur == NULL || cur == cur1 )	// oldest or same vintage?
				goto done_alloc2;				// not possible to allocate more

			k = VS( cur, "__dLdVint" );			// addt'l labor demand of vint.
		}

		cur2 = SEARCHS( cur1, "WrkV" );			// pick old vint. first worker
		move_worker( SHOOKS( cur2 ), cur, vint_learn );// move worker to vintage
		DELETE( cur2 );							// remove old bridge-object
		++i;
		--k;									// update new vintage demand
		--j;									// one worker less in old vint.
	}
}

done_alloc2:

RESULT( i * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_c2" )
/*
Planned average unit cost of firm in consumption-good sector
Considers only used machines, differently from original K+S
Also updates '_A2', '_A2p'
*/

v[1] = V( "_Q2d" );								// desired production
v[2] = VL( "_K", 1 );							// available capital stock
v[3] = VS( PARENT, "m2" );						// machine output per period
v[4] = VL( "_w2avg", 1 );						// average firm wage

// number of unused machines, max should be total-1 to compute cost/productivity
v[5] = max( floor( v[2] / v[3] ) - ceil( max( v[1], v[3] ) / v[3] ), 0 );

// scan all vintages, from oldest to newest, preferring to use newer ones
v[0] = v[6] = v[7] = v[8] = 0;					// accumulators
CYCLE( cur, "Vint" )							// choose vintages to use
{
	v[9] = VS( cur, "__nVint" );				// number of machines in vintage

	if ( v[5] >= v[9] )							// none to be used in the vint.?
	{
		v[5] -= v[9];							// less machines not to use
		v[12] = 0;								// no machine to use in vintage
	}
	else
	{
		v[10] = VS( cur, "__Avint" );			// vintage notional productivity
		v[11] = VLS( cur, "__AeVint", 1 );		// vintage effective product.
		v[6] += v[12] = v[9] - v[5];			// add to be used machines
		v[7] += v[12] * v[10];					// add notional productivities
		v[8] += v[12] * v[11];					// add effective productivities
		v[0] += v[12] * v[4] / v[11];			// add used machines cost
		v[5] = 0;								// no more machine not to use

		if ( v[1] == 0 )
			v[12] = 0;							// no worker if no production
	}

	WRITES( cur, "__toUseVint", v[12] );		// number mach. to try to use
}

if ( v[6] == 0 )								// no machine?
{
	V( "_supplier" );							// ensure supplier is selected
	cur1 = PARENTS( SHOOKS( HOOK( SUPPL ) ) );	// pointer to supplier

	v[6] = 1;									// 1 notional machine
	v[7] = v[8] = VS( cur1, "_Atau" );			// new machines productivity
	v[0] = v[4] / v[7];							// machine unit cost
}

WRITE( "_A2p", v[7] / v[6] );					// potential productivity
WRITE( "_A2", v[8] / v[6] );					// expected productivity

RESULT( v[0] / v[6] )


EQUATION( "_f2" )
/*
Market share of firm in consumption-good sector
It is computed using a replicator equation over the relative competitiveness
of the firm
Because of entrants, market shares may add-up to more than one, so 'f2rescale'
must be used before '_f2' is used, by calling 'CPI'
*/

v[1] = VS( PARENT, "f2min" );					// minimum share to stay

switch( ( int ) V( "_life2cycle" ) )			// entrant firm state
{
	case 0:										// non-producing entrant
	default:									// exiting incumbent
		END_EQUATION( 0 );

	case 1:										// first-period entrant
		v[0] = VL( "_K", 1 ) / VLS( PARENT, "K", 1 );// same as capital share
		END_EQUATION( max( v[0], v[1] ) );		// but over minimum

	case 2:										// 2nd-4th-period entrant
		// replicator equation
		v[0] = VL( "_f2", 1 ) * ( 1 + VS( PARENT, "chi" ) *
								  ( V( "_E" ) / VS( PARENT, "Eavg" ) - 1 ) );
		v[0] = max( v[0], v[1] );				// but over minimum
		break;

	case 3:										// incumbent
		// replicator equation
		v[0] = VL( "_f2", 1 ) * ( 1 + VS( PARENT, "chi" ) *
								  ( V( "_E" ) / VS( PARENT, "Eavg" ) - 1 ) );
}

// lower-bounded slightly below (multi-period) exit threshold
// to ensure firm sells production before leaving the market
RESULT( max( v[0], 0.99 * v[1] / VS( PARENT, "n2" ) ) )


EQUATION( "_fires2" )
/*
Number of workers fired by firm in consumption-good sector
Process required firing using the appropriate rule
*/

if ( V( "_life2cycle" ) == 0 )					// entrant firm?
	END_EQUATION( 0 );

v[1] = max( V( "_Q2pe" ) - V( "_Q2d" ), 0 );	// expected extra capacity

// check if would fire too many workers because of scaling (# of "modules")
v[2] = VS( LABSUPL2, "Lscale" ) * V( "_A2" );	// prod.-adjusted module size
v[1] = floor( v[1] / v[2] ) * v[2];				// rounded down capacity shrink

// pick the appropriate firing rule
int fRule = V( "_postChg" ) ? VS( GRANDPARENT, "flagFireRuleChg" ) :
							  VS( GRANDPARENT, "flagFireRule" );
switch ( fRule )
{
	case 0:										// never fire (exc. retirement)
	case 1:										// never fire with sharing
	default:
		v[0] = 0;
		break;

	case 2:										// only fire if firm downsizing
		// production being reduced and extra capacity is expected?
		if ( V( "_dQ2d" ) < 0 && v[1] > 0 )		// workers have to be fired?
			v[0] = fire_workers( var, THIS, MODE_ADJ, v[1], &v[2] );
		else
			v[0] = 0;
		break;

	case 3:										// only fire if firm at losses
		// production being reduced and extra capacity is expected?
		if ( VL( "_Pi2", 1 ) < 0 && v[1] > 0 )	// workers have to be fired?
			v[0] = fire_workers( var, THIS, MODE_ADJ, v[1], &v[2] );
		else
			v[0] = 0;
		break;

	case 4:										// fire if payback is achieved
		// fire insufficient payback workers
		v[0] = fire_workers( var, THIS, MODE_PBACK, v[1], &v[2] );
		break;

	case 5:										// fire when contract ends
		// fire all workers with finished contracts
		v[0] = fire_workers( var, THIS, MODE_ALL, v[1], &v[2] );
		break;

	case 6:										// reg. 5 until t=T, then reg. Y
		// fire non needed, non stable workers
		v[0] = fire_workers( var, THIS, MODE_IPROT, v[1], &v[2] );
}

RESULT( v[0] )


EQUATION( "_mu2" )
/*
Mark-up of firm in consumption-good sector
*/

v[1] = VL( "_f2", 1 );							// past periods market shares
v[2] = VL( "_f2", 2 );
v[3] = VS( PARENT, "f2min" );					// market exit share threshold

if ( v[1] < v[3] || v[2] < v[3] )				// just entered firms keep it
	END_EQUATION( CURRENT );

RESULT( CURRENT * ( 1 + VS( PARENT, "upsilon" ) * ( v[1] / v[2] - 1 ) ) )


EQUATION( "_q2" )
/*
Product quality of a firm in consumption-good sector
Equal to the average of the log tenure skills of workers
*/

if ( V( "_life2cycle" ) == 0 )
	END_EQUATION( VLS( PARENT, "q2avg", 1 ) );

V( "_L2" );										// ensure hiring is done

v[0] = i = 0;									// accumulators
CYCLE( cur, "Wrk2" )
{
	v[0] += VS( SHOOKS( cur ), "_sT" );
	++i;
}

RESULT( i > 0 ? v[0] / i : 1 )


EQUATION( "_supplier" )
/*
Selected machine supplier by firm in consumption-good sector
Also set firm 'hook' pointers to supplier firm object
*/

VS( CAPSECL2, "inn" );							// ensure innovation is done and
												// brochures distributed
v[1] = VS( PARENT, "m2" );						// machine modularity
v[2] = V( "_postChg" ) ? VS( PARENT, "bChg" ) : VS( PARENT, "b" );// req payback
v[3] = VL( "_w2avg", 1 );						// average firm wage

v[4] = DBL_MAX;									// supplier price/cost ratio
i = 0;
cur2 = cur3 = NULL;
CYCLE( cur, "Broch" )							// use brochures to find supplier
{
	cur1 = PARENTS( SHOOKS( cur ) );			// pointer to supplier object

	// compare total machine unit cost (acquisition + operation for payback period)
	v[5] = VS( cur1, "_p1" ) / v[1] + v[2] * v[3] / VS( cur1, "_Atau" );
	if ( v[5] < v[4] )							// best so far?
	{
		v[4] = v[5];							// save current best supplier
		i = VS( cur1, "_ID1" );					// supplier ID
		cur2 = SHOOKS( cur );					// own entry on supplier list
		cur3 = cur;								// best supplier brochure
	}
}

// if supplier is found, simply update it, if not, draw a random one
if ( cur2 != NULL && cur3 != NULL )
{
	WRITES( cur2, "__tSel", T );				// update selection time
	WRITE_HOOK( SUPPL, cur3 );					// pointer to current brochure
}
else											// no brochure received
{
	cur1 = set_supplier( THIS );				// draw new supplier
	i = VS( cur1, "_ID1" );
}

RESULT( i )


EQUATION( "_w2o" )
/*
Wage offer to workers in queue of firm in consumption-good sector
*/

if ( VS( GRANDPARENT, "flagHeterWage" ) == 0 )	// centralized wage setting?
{
	v[0] = VS( LABSUPL2, "wCent" );				// single wage centrally defined
	goto end_offer;
}

v[9] = VLS( PARENT, "w2oAvg", 1 );				// average market offer
v[13] = VS( LABSUPL2, "wCap" );					// wage cap multiplier
h = V( "_life2cycle" );							// firm status
k = V( "_postChg" ) ? VS( GRANDPARENT, "flagWageOfferChg" ) :
					  VS( GRANDPARENT, "flagWageOffer" );

if ( k == 0 )									// wage premium mode?
{
	if ( h == 0 )								// if entrant
		v[0] = v[9];							// use market average as base
	else
		v[0] = CURRENT;

	switch ( ( int ) VS( GRANDPARENT, "flagWagePremium" ) )
	{											// define wage premium type
		case 0:									// no premium
		default:
			break;

		case 1:									// indexed premium (WP1)
			v[1] = VS( LABSUPL2, "psi1" );		// inflation adjust. parameter
			v[2] = VS( LABSUPL2, "psi2" );		// general prod. adjust. param.
			v[3] = VS( LABSUPL2, "psi3" );		// unemploym. adjust. parameter
			v[4] = VS( LABSUPL2, "psi4" );		// firm prod. adjust. parameter
			v[11] = VS( LABSUPL2, "psi5" );		// firm vacancy booster param.
			v[5] = VLS( PARENT, "dCPIb", 1 );	// inflation variation
			v[6] = VLS( GRANDPARENT, "dAb", 1 );// general productivity var.
			v[7] = VLS( LABSUPL2, "dUeB", 1 );	// unemployment variation
			v[12] = VL( "_L2vac", 1 );			// previous vacancy rate

			// notional productivity variation (firm), consider entrants
			v[8] = ( h == 0 ) ? 0 : VL( "_dA2b", 1 );

			// make sure total productivity effect is bounded to 1
			if ( ( v[2] + v[4] ) > 1 )
				v[2] = max( 1 - v[4], 0 );		// adjust general prod. effect

			v[0] *= 1 + v[1] * v[5] + v[2] * v[6] + v[3] * v[7] +
					v[4] * v[8] + v[11] * v[12];
			break;

		case 2:									// endogenous mechanism (WP2)
			if ( v[0] != 0 )					// valid  offer last period?
				v[0] *= 1 + max( v[9] / v[0] - 1, 0 );
			else								// no: use market average
				v[0] = v[9];
	}
}
else
{												// lowest wage mode
	VS( LABSUPL2, "appl" );						// ensure applications are done
	j = ceil( ( V( "_L2d" ) - VL( "_L2", 1 ) ) * // number of workers (scaled)
			  ( 1 + VS( LABSUPL2, "theta" ) ) / VS( LABSUPL2, "Lscale" ) );
	j = max( j, 1 );							// minimum one worker for calc.

	// sort firm's candidate list according to the defined strategy
	int hOrder = V( "_postChg" ) ? VS( GRANDPARENT, "flagHireOrder2Chg" ) :
								   VS( GRANDPARENT, "flagHireOrder2" );
	order_applications( hOrder, & V_EXT( firm2E, appl ) );

	// search applications set (increasing wage requests) for enough workers
	i = 0;										// workers counter
	v[0] = 0;									// highest wage found
	appLisT::iterator its;
	CYCLE_EXT( its, firm2E, appl )				// run over enough applications
	{
		if ( its->w > v[0] )					// new high wage request?
			v[0] = its->w;						// i-th worker wage

		if ( ++i >= j )
			break;								// stop when enough workers
	}

	if ( v[0] == 0 )							// no worker in queue
		v[0] = CURRENT;							// keep current offer
}

// check for abnormal change
if ( h > 0 && v[13] > 0 )
{
	v[14] = v[0] / CURRENT;						// calculate multiple
	v[15] = v[14] < 1 ? 1 / v[14] : v[14];

	if ( v[15] > v[13] )						// explosive change?
		v[0] = v[14] < 1 ? CURRENT / v[13] : CURRENT * v[13];
}

// check if non-entrant firm is able to pay wage
if ( h > 0 )
{
	v[10] = VL( "_p2", 1 ) * VL( "_A2", 1 );
												// max wage for minimum markup
	if ( v[10] > 0 && v[0] > v[10] )			// over max?
		v[0] = v[10];
	else
		if ( v[10] <= 0 )						// max can't be calculated?
			v[0] = min( v[0], CURRENT );		// limit to current
}

// under unemployment benefit or minimum wage? Adjust if necessary
v[0] = max( v[0], max( VS( LABSUPL2, "wU" ), VS( LABSUPL2, "wMinPol" ) ) );

end_offer:

// save offer in global offers set
wageOffer woData;
woData.offer = v[0];
woData.workers = VL( "_L2", 1 );
woData.firm = THIS;

EXEC_EXTS( GRANDPARENT, countryE, firm2wo, push_back, woData );

RESULT( v[0] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_CI" )
/*
Canceled investment of firm in consumption-good sector
*/

V( "_supplier" );								// ensure new supplier is set

cur = HOOK( SUPPL );							// pointer to current supplier
VS( PARENTS( SHOOKS( cur ) ), "_Q1e" );			// make sure supplier produced

v[1] = VS( SHOOKS( cur ), "__nCan" );			// canceled machine number
k = VS( SHOOKS( cur ), "__tOrd" );				// time of canceled order

if ( k == T && v[1] > 0 )
{
	v[2] = VS( PARENT, "m2" );					// machine output per period
	v[3] = V( "_SI" ) / v[2];					// machines to substitute
	v[4] = V( "_EI" ) / v[2];					// machines to expand
	v[5] = VS( PARENTS( SHOOKS( cur ) ), "_p1" );// machine price

	if ( v[1] > v[3] )							// no space for substitution?
	{
		WRITE( "_SI", 0 );						// reset substitution investment
		WRITE( "_EI", ( v[4] - v[1] + v[3] ) * v[2] );// remaining expansion inv.
	}
	else
		WRITE( "_SI", ( v[3] - v[1] ) * v[2] );	// shrink substitution investm.

	update_depo( THIS, v[5] * v[1], true );		// recover paid machines value

	v[0] = v[1] * v[2];							// canceled investment
}
else
	v[0] = 0;

RESULT( v[0] )


EQUATION( "_D2d" )
/*
Desired (potential) demand for firm in consumption-good sector
*/
VS( PARENT, "CPI" );							// ensure m.s. updated
RESULT( V( "_f2" ) * VS( PARENT, "D2d" ) )


EQUATION( "_Inom" )
/*
Investment (nominal/currency terms) of firm in consumption-good sector
*/

V( "_K" );										// ensure capital is deployed

cur = HOOK( TOPVINT );							// last capital vintage

if ( cur != NULL && VS( cur, "__tVint" ) == T )	// capital deployed in period?
	v[0] = VS( cur, "__nVint" ) * VS( cur, "__pVint" );
else
	v[0] = 0;

RESULT( v[0] )


EQUATION( "_JO2" )
/*
Open job positions for a firm in consumption-good sector
*/

VS( CAPSECL2, "hires1" );						// ensure sector 1 is done
V( "_fires2" );									// and fires are also done

v[1] = V( "_L2d" ) - COUNT( "Wrk2" ) * VS( LABSUPL2, "Lscale" );

RESULT( max( ceil( ( 1 + VS( LABSUPL2, "theta" ) ) * v[1] ), 0 ) )


EQUATION( "_K" )
/*
Capital employed by firm in consumption-good sector
*/

V( "_CI" );										// ensure cancel investment done

v[1] = VS( PARENT, "m2" );						// machine output per period
v[2] = V( "_SI" );								// substitution investment
v[3] = V( "_EI" );								// expansion investment

// new machines delivered only if supplier has not canceled the order
if ( v[2] + v[3] > 0 )
{
	v[4] = floor( ( v[2] + v[3] ) / v[1] );		// total number of new machines

	if ( v[4] > 0 )								// new machines to install?
		add_vintage( var, THIS, v[4], false );	// create vintage
}

v[5] = max( VL( "_K", 1 ) + v[3] - V( "_Kd" ), 0 );// desired capital shrinkage
v[6] = floor( v[5] / v[1] );					// machines to remove from K

v[7] = floor( v[2] / v[1] );					// machines to substitute in K

j = T + 1;										// oldest vintage so far
h = 0;											// oldest vintage ID
CYCLE_SAFE( cur, "Vint" )						// search from older vintages
{
	v[8] = VS( cur, "__RSvint" );				// number of machines to scrap

	if ( v[8] < 0 )								// end-of-life vintage to scrap?
	{
		v[8] = - v[8];							// absolute vintage size

		if ( v[6] > 0 )							// yet capital to shrink?
		{
			if ( v[8] > v[6] )					// more than needed?
			{
				v[8] -= v[6];					// just reduce vintage
				v[6] = 0;						// shrinkage done
				WRITES( cur, "__nVint", v[8] );
			}
			else								// scrap entire vintage
			{
				if ( scrap_vintage( var, cur ) >= 0 )// not last vintage?
				{
					v[6] -= v[8];
					continue;					// don't consider for old vint.
				}
				else
				{
					v[8] = 0;					// last: nothing else to scrap
					v[6] -= v[8] - 1;
				}
			}
		}
	}

	// something yet to scrap and substitution to be done?
	if ( v[8] > 0 && v[7] > 0 )
	{
		if ( v[8] > v[7] )						// more than needed?
		{
			v[8] -= v[7];						// just reduce vintage
			v[7] = 0;							// substitution done
			WRITES( cur, "__nVint", v[8] );
		}
		else									// scrap entire vintage
		{
			if ( scrap_vintage( var, cur ) >= 0 )// not last vintage?
			{
				v[7] -= v[8];
				continue;						// don't consider for old vint.
			}
		}
	}

	i = VS( cur, "__tVint" );					// time of vintage install
	if ( i < j )								// oldest so far?
	{
		j = i;
		h = VS( cur, "__IDvint" );				// save oldest
	}
}

WRITE( "_oldVint", h );

RESULT( SUM( "__nVint" ) * v[1] )


EQUATION( "_Knom" )
/*
Current capital (nominal/currency terms) of firm in consumption-good sector
*/
V( "_K" );										// ensure capital is deployed
RESULT( WHTAVE( "__nVint", "__pVint" ) )


EQUATION( "_L2" )
/*
Effective (absolute) number of workers of firm in consumption-good sector
Result is scaled according to the defined scale
*/
VS( PARENT, "hires2" );							// make sure hiring done
RESULT( COUNT( "Wrk2" ) * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_L2d" )
/*
Labor demand of firm in consumption-good sector
*/
RESULT( V( "_life2cycle" ) > 0 ? ceil( V( "_Q2" ) / V( "_A2" ) ) : 0 )


EQUATION( "_L2vac" )
/*
Net vacancy rate of labor (unfilled positions over labor employed) for firm in
consumption-good sector
*/

v[1] = V( "_L2" );								// current number of workers
v[2] = V( "_JO2" );								// current open positions

if( v[1] == 0 )									// firm has no worker?
	v[0] = ( v[2] == 0 ) ? 0 : 1;				// handle limit case
else
	v[0] = min( v[2] / v[1], 1 );				// or calculate the regular way

RESULT( v[0] )


EQUATION( "_N" )
/*
Inventories (unsold output) of firm in consumption-good sector
*/

VS( PARENT, "D2" );								// ensure demand is allocated

v[0] = CURRENT + V( "_Q2e" ) - V( "_D2" );

RESULT( ROUND( v[0], 0, 0.001 ) )				// avoid rounding errors on zero


EQUATION( "_Pi2rate" )
/*
Profit rate (profits over capital) of firm in consumption-good sector
*/
v[1] = VL( "_K", 1 );							// available capital
RESULT( v[1] > 0 ? V( "_Pi2" ) / v[1] : 0 )		// ignore no capital firms


EQUATION( "_Pi2" )
/*
Profit of firm (before taxes) in consumption-good sector
*/
RESULT( V( "_S2" ) + V( "_iD2" ) - V( "_W2" ) - V( "_i2" ) )


EQUATION( "_Q2e" )
/*
Effective output of firm in consumption-good sector
*/
RESULT( min( V( "_Q2" ), V( "_Q2p" ) ) )


EQUATION( "_Q2p" )
/*
Potential production with current machines and workers for a firm in
consumption-good sector
*/
RESULT( SUM( "__Qvint" ) )


EQUATION( "_Q2pe" )
/*
Expected potential production with remaining workers for a firm in
consumption-good sector
*/

V( "_quits2" );									// ensure quit/retiring is done

v[0] = 0;										// accumulator
CYCLE( cur, "Wrk2" )
	v[0] += VLS( SHOOKS( cur ), "_Q", 1 );		// add previous production

RESULT( v[0] * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_Q2u" )
/*
Capacity utilization for a firm in consumption-good sector
*/
v[1] = V( "_Q2p" );
RESULT( v[1] > 0 ? V( "_Q2e" ) / v[1] : VLS( PARENT, "Q2u", 1 ) )


EQUATION( "_S2" )
/*
Sales of firm in consumption-good sector
*/
VS( PARENT, "D2" );								// ensure demand is allocated
RESULT( V( "_p2" ) * V( "_D2" ) )


EQUATION( "_SId" )
/*
Desired substitution investment of firm in consumption-good sector
*/

v[1] = VS( PARENT, "m2" );						// machine output per period

v[2] = 0;										// scrapped machine accumulator
CYCLE( cur1, "Vint" )							// search last vintage to scrap
{
	v[3] = VS( cur1, "__RSvint" );				// number of machines to scrap

	if ( v[3] == 0 )							// nothing else to do
		break;

	v[2] += abs( v[3] );						// accumulate vintage
}

v[4] = max( VL( "_K", 1 ) - V( "_Kd" ), 0 );	// capital shrinkage desired?
v[5] = floor( v[4] / v[1] );					// machines to remove from K

RESULT( max( v[2] - v[5], 0 ) * v[1] )


EQUATION( "_W2" )
/*
Total wages paid by firm in sector 2
*/

V( "_L2" );										// ensure hiring is done

v[0] = 0;										// wage accumulator
CYCLE( cur, "Wrk2" )
	v[0] += VS( SHOOKS( cur ), "_w" );			// adding wages

RESULT( v[0] * VS( LABSUPL2, "Lscale" ) )		// consider labor scaling


EQUATION( "_c2e" )
/*
Effective average unit cost of firm in consumption-good sector
Use expected cost if firm is not producing
*/
v[1] = V( "_Q2e" );
RESULT( v[1] > 0 ? V( "_W2" ) / v[1] : V( "_c2" ) )


EQUATION( "_dA2b" )
/*
Notional productivity (bounded) rate of change of firm in consumption-good sector
Used for wages adjustment only
*/
RESULT( mov_avg_bound( THIS, "_A2", VS( GRANDPARENT, "mLim" ),
					   VS( GRANDPARENT, "mPer" ) ) )


EQUATION( "_dNnom" )
/*
Change in firm's nominal (currency terms) inventories for a firm in
consumption-good sector
*/
RESULT( V( "_p2" ) * V( "_N" ) - VL( "_p2", 1 ) * VL( "_N", 1 ) )


EQUATION( "_dQ2d" )
/*
Desired production change (absolute) for a firm in consumption-good sector
*/
RESULT( V( "_Q2d" ) - VL( "_Q2e", 1 ) )


EQUATION( "_i2" )
/*
Interest paid by firm in consumption-good sector
*/
RESULT( VL( "_Deb2", 1 ) * VLS( FINSECL2, "rDeb", 1 ) *
		( 1 + ( VL( "_qc2", 1 ) - 1 ) * VS( FINSECL2, "kConst" ) ) )


EQUATION( "_iD2" )
/*
Interest received from deposits by firm in consumption-good sector
*/
RESULT( VL( "_NW2", 1 ) * VLS( FINSECL2, "rD", 1 ) )


EQUATION( "_life2cycle" )
/*
Stage in life cycle of firm in consumer-good sector:
0 = pre-operational entrant firm
1 = operating entrant firm (first period producing)
2.x = operating entrant firm (x=2nd-4th period producing)
3 = incumbent firm
4 = exiting firm
*/

switch( ( int ) CURRENT )
{
	case 0:										// pre-operational entrant
		if ( VL( "_K", 1 ) > 0 )				// firm has capital available?
			v[0] = 1;							// turn into operational entrant
		else
			v[0] = 0;							// keep as pre-op. entrant
		break;

	case 1:										// operating entrant
		v[0] = 2.2;								// turn into running entrant
		break;

	case 2:										// running entrant
		v[0] = CURRENT + 0.1;
		if ( v[0] > 2.4001 )					// handle rounding error
			v[0] = 3;							// turn into incumbent
		break;

	default:									// incumbent/exiting
		v[0] = CURRENT;							// keep as it is
		PARAMETER;								// but no longer compute
}

RESULT( v[0] )


EQUATION( "_p2" )
/*
Price of good of firm in consumption-good sector
Entrants notional price are the market average
*/
RESULT( ( 1 + V( "_mu2" ) ) * ( V( "_life2cycle" ) == 0 ?
								VLS( PARENT, "c2", 1 ) : V( "_c2" ) ) )


EQUATION( "_quits2" )
/*
Number of workers quitting jobs (not fired) in period for firm in sector 2
Updated in 'hires1' and 'hires2'
*/

V( "_retires2" );								// ensure retiring is done

if ( VS( GRANDPARENT, "flagGovExp" ) < 2 )		// unemployment benefit exists?
	END_EQUATION( 0 );

v[1] = VS( LABSUPL2, "wU" );					// unemployment benefit in t

h = 0;
CYCLE_SAFE( cur, "Wrk2" )
	if ( VS( SHOOKS( cur ), "_w" ) <= v[1] )	// under unemp. benefit?
	{
		fire_worker( var, SHOOKS( cur ) );
		++h;
	}

RESULT( h * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_retires2" )
/*
Number of workers retiring from jobs (not fired) in period for firm in sector 2
*/

if ( VS( LABSUPL2, "Tr" ) == 0 )				// retirement disabled?
	END_EQUATION( 0 )

h = 0;
CYCLE_SAFE( cur, "Wrk2" )
	if ( VS( SHOOKS( cur ), "_age" ) == 1 )		// is a "reborn"?
	{
		fire_worker( var, SHOOKS( cur ) );
		++h;
	}

RESULT( h * VS( LABSUPL2, "Lscale" ) )


EQUATION( "_sT2min" )
/*
Minimum workers tenure skills of a firm in consumption-good sector
*/

if ( VS( GRANDPARENT, "flagWorkerLBU" ) <= 1 )	// no learning by tenure mode?
	END_EQUATION( INISKILL );

V( "_L2" );										// ensure hiring is done

v[0] = DBL_MAX;									// current minimum
CYCLE( cur, "Wrk2" )
{
	v[1] = VS( SHOOKS( cur ), "_sT" );

	if ( v[1] < v[0] )
		v[0] = v[1];							// keep minimum
}

RESULT( v[0] < DBL_MAX ? v[0] : CURRENT )		// handle the no worker case


EQUATION( "_s2avg" )
/*
Weighted average workers compound skills of a firm in consumption-good sector
*/

if ( VS( GRANDPARENT, "flagWorkerLBU" ) == 0 )	// no learning ?
	END_EQUATION( INISKILL );

V( "_L2" );										// ensure hiring is done

v[0] = i = 0;									// accumulators
CYCLE( cur, "Wrk2" )
{
	v[0] += VS( SHOOKS( cur ), "_s" );
	++i;
}

RESULT( i > 0 ? v[0] / i : INISKILL )			// handle the no worker case


EQUATION( "_w2avg" )
/*
Average wage paid by firm in consumption-good sector
*/

V( "_L2" );										// ensure hiring is done

v[0] = i = 0;									// accumulators
CYCLE( cur, "Wrk2" )
{
	v[0] += VS( SHOOKS( cur ), "_w" );
	++i;
}

RESULT( i > 0 ? v[0] / i : VLS( PARENT, "w2avg", 1 ) )


EQUATION( "_w2realAvg" )
/*
Average real wage paid by firm in consumption-good sector
*/
RESULT( V( "_w2avg" ) / VS( PARENT, "CPI" ) )


/*========================== SUPPORT LSD FUNCTIONS ===========================*/

EQUATION( "_CS2a" )
/*
Bank credit supply available (new debt) to firm in consumer-good sector
Function called multiple times in single time step
*/

v[1] = V( "_Deb2" );							// current firm debt
v[2] = V( "_Deb2max" );							// maximum prudential credit

if ( v[2] > v[1] )								// more credit possible?
{
	v[0] = v[2] - v[1];							// potential free credit

	cur = HOOK( BANK );							// firm's bank
	v[3] = VS( cur, "_TC2free" );				// bank's available credit

	if ( v[3] > -0.1 )							// credit limit active
		v[0] = min( v[0], v[3] );				// take just what is possible
}
else
	v[0] = 0;									// no credit available

RESULT( v[0] )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "_A2", "" )
/*
Machine-level average labor productivity of firm in consumption-good sector
Updated in '_c2'
*/

EQUATION_DUMMY( "_A2p", "_c2" )
/*
Machine-level average potential labor productivity of firm in consumption-good
sector
Updated in '_c2'
*/

EQUATION_DUMMY( "_CD2", "" )
/*
Credit demand for firm in consumption-good sector
Updated in '_Deb2max', '_Q2', '_EI', '_SI', '_Tax2'
*/

EQUATION_DUMMY( "_CD2c", "" )
/*
Credit demand constraint for firm in consumption-good sector
Updated in '_Deb2max', '_Q2', '_EI', '_SI', '_Tax2'
*/

EQUATION_DUMMY( "_CS2", "" )
/*
Credit supplied to firm in consumption-good sector
Updated in '_Deb2max', '_Q2', '_EI', '_SI', '_Tax2'
*/

EQUATION_DUMMY( "_D2", "" )
/*
Demand fulfilled by firm in consumption-good sector
Updated in 'D2'
*/

EQUATION_DUMMY( "_Deb2", "" )
/*
Stock of bank debt of firm in consumption-good sector
Updated in '_Q2', '_EI', '_SI', '_Tax2'
*/

EQUATION_DUMMY( "_NW2", "" )
/*
Net worth of firm in consumption-good sector
Updated in '_Q2', '_EI', '_SI', '_Tax2'
*/

EQUATION_DUMMY( "_NW2p", "_Q2" )
/*
Provision for production of firm in consumption-good sector
Updated in '_Q2'
*/

EQUATION_DUMMY( "_hires2", "" )
/*
Effective number of workers hired in period for firm in sector 2
Updated in 'hires2'
*/

EQUATION_DUMMY( "_l2", "" )
/*
Unfilled demand of firm in consumption-good sector
Updated in 'D2'
*/

EQUATION_DUMMY( "_oldVint", "_K" )
/*
Oldest vintage (ID) in use in period by firm in consumption-good sector
Updated in '_K'
*/

EQUATION_DUMMY( "_qc2", "" )
/*
Credit class of firm in sector 2 (1,2,3,4)
Updated in 'cScores'
*/
