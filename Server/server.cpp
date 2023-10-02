#include <iostream>

#include "service/CalendarService.hpp"
#include "service/NotifierService.hpp"
#include "utils/ParseUtils.hpp"

int main(int argc, char** argv) {
    shablov::DBService db{"test.sqlite"};
    shablov::CalendarService calendar{db, 50051};
    shablov::NotifierService notifier{db, 50052};

    return 0;
}
