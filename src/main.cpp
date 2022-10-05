#include <iostream>
#include <libtorrent/session.hpp>

int main(int argc, char* argv[])
{
    std::cout << argv[1];

    libtorrent::session session;

    return 0;
}
