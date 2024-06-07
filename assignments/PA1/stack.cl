(*
 *  CS164 Fall 94
 *
 *  Programming Assignment 1
 *    Implementation of a simple stack machine.
 *
 *  Skeleton file
 *)


class Stack {
	s : String;
	init() : Stack {
		{
			s <- "";
			self;
		}
	};

	push(new_s : String) : Object{
		{
			s <- s.concat(new_s);
		}
	};

	pop() : String {
		{
			let top : String <- s.substr(s.length() - 1, 1) in {
				s <- s.substr(0, s.length() - 1);
				top;
			};
		}
	};

	-- Display from top of stack to bottom
	display() : Object {
		{
			let io : IO <- new IO,
				i : Int <- s.length() - 1 in {
				while (0 <= i) loop 
					{
						io.out_string(s.substr(i,1).concat("\n"));
						i <- i - 1;
					}
				pool;
			};
		}
	};

};

class Main inherits IO {

	i : Int <- 3;
	stack : Stack <- (new Stack).init();
	main() : Object {
		{
			out_string("Hello\n");
			stack.display();
			stack.push("h");
			stack.push("e");
			stack.display();
			stack.display();
		}
	};

};
