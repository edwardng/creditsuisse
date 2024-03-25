# Environment

Developed and tested solely only on Ubuntu 22.04 / CLion / GCC 11.2

# Compiling and Building

1. create a build directory as `~/build` and `cd` into this directory
2. run cmake `target_directory` where `target_directory` is the directory of the project root
3. run `make`
4. libME_LIB.a should be built

# Quick discussion on the design

- Extensibility is the center of this matching engine design;
- Stability and correctness is of paramount importance;
- This implementation adheres serving the purposes of back-testing, not industry production / dark pool.

## Validators

Currently support 3 types of validators:

- New order request validators
- Cancel order request validators
- Matching validators

User can plug custom validation logic into the matching algo via these validators interfaces.

Validation result drives how the matching algo behaves - such as continue or stop the order request from being (further)
processed.

Any number of these validators are passed into the matching algo as template parameters.

## Notable extra mile

An order specified with minimum execution quantity larger than 0 means the order must be matched with execution having
quantity greater than or equal to the minimum execution quantity.

Minimum execution quantity is supported by some exchanges but not all, so we make this a custom plugin.

A minimum execution quantity plugin is implemented with proper test coverage.

This implementation demonstrates how easy it is in implementing additional requirements such as this minimum execution
quantity:

- A user custom type with fields storing the necessary values,
- A simple minimum execution validator
- Plug the validator into matching algo as a template parameter

In the same way, repeated client order ID in order entry or no such order in cancel order request can all be addressed
with the same validator interface approach.

These code demonstrates how easy it is to extend / adopt to different exchanges with different requirements.

## Order Amendment

In the same way, we can easily extend the matching algo to support order amendment with

- additional amendment validators
- Validators can help drive the amendment behaviour with `ValidationResponse` whether to perform native amend (such as
  amend volume down) or update the order as cancel-new.

Order amendment is not implemented in this exercise not because of the code complexity but extensive testing
requirements,
as we need to ensure it works altogether with different complex scenarios such as amend down partial filled orders,
and crossover scenarios.

## Segregation of Matching Algo / Data / Matching Engine

This is to support any requirement changes as well as effective automated testing.
`PriceTimePriorityMatching` is just one possible way of matching.
This design provides the possibility to easily extend to cover with requirements such as equilibrium auction matching,
user can simply implement the IMatchingAlgo interface and have a MatchingEngine to manage it through.

# Matching Engine

Matching engine often get a mandate to guarantee Price-Time order request is honoured (first come first serves).
The implementation ensures exactly the same thread is always employed to process one instrument order queue.

With limited cores on a machine, one thread may be assigned to multiple instruments.

We assumed all instruments trading is fairly scattered but uniform distributed, that is, no attempt has been made
to automatic rebalance workloads between different threads if execution concentrates on selected instruments.

# Other considerations

- Within matching engine, boost SPSC lock free queue was not adopted as I want to keep the flexibility of extending the
  order queue on demand, when necessary.
- The implementation is provided as a library instead of an application since it requires user to customize the engine
  with code.
- For `default` demonstration that we can make the matching engine runs, we can refer to
  test/engine/matching_engine_test.cpp as well as other tests.
- Best effort in coverage test within designated time frame