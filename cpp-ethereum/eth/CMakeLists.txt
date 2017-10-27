set(
    sources
    AccountManager.cpp AccountManager.h
    main.cpp
    MinerAux.h
)

add_executable(eth ${sources})
target_link_libraries(eth PRIVATE ethereum ethashseal evm web3jsonrpc webthree devcore contract)

install(TARGETS eth DESTINATION bin)
