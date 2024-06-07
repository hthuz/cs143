(*
 *  CS164 Fall 94
 *
 *  Programming Assignment 1
 *    Implementation of a simple stack machine.
 *
 *  Skeleton file
 *)



class ListNode{
	s : String;
	next : ListNode;
	init(new_s: String, new_next: ListNode) : ListNode{
		{
			s <- new_s;
			next <- new_next;
			self;
		}
	};

	getValue() : String {s};
	getNext() : ListNode {next};
	setNext(new_next: ListNode) : Object {
		next <- new_next
	};
};

class Stack {
	
	dummy : ListNode;
	length : Int;
	nil : ListNode;

	init() : Stack {
		{
			dummy <- (new ListNode).init("dummy", nil);
			length <- 0;
			self;
		}
	};

	push(new_s : String) : Object{
		{
			let old_head : ListNode <- dummy.getNext(),
				new_head : ListNode <- (new ListNode).init(new_s, old_head) in {
				dummy.setNext(new_head);
			};
			length <- length + 1;
		}
	};

	length() : Int {
		length
	};

	getTop() : String {
		dummy.getNext().getValue()
	};

	pop() : String {
		{
			let old_head : ListNode <- dummy.getNext(),
				new_head : ListNode <- old_head.getNext() in {
				dummy.setNext(new_head);
				length <- length - 1;
				old_head.getValue();
			};
		}
	};

	-- Display from top of stack to bottom
	display() : Object {
		{
			let io : IO <- new IO,
				node : ListNode <- dummy.getNext() in {
				while (not (isvoid node)) loop 
					{
						io.out_string(node.getValue().concat("\n"));
						node <- node.getNext();
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
			out_string(">");

			let pushCommand: PushCommand <- new PushCommand,
				evaluateCommand: EvaluateCommand <- new EvaluateCommand,
				displayCommand: DisplayCommand <- new DisplayCommand,
				input: String <- in_string() in {
				while ( not (input = "x")) loop 
				{
					if (input = "e") then
						evaluateCommand.execute(stack, input)
					else
						if (input = "d") then
							displayCommand.execute(stack, input)
						else
							pushCommand.execute(stack, input)
						fi
					fi;

					out_string(">");
					input <- in_string();
				}
				pool;
			};

		}
	};

};

class StackCommand {

	execute(stack : Stack, input: String) : Object {
		{
			stack.push(input);
		}
	};
};

class PushCommand inherits StackCommand {

};

class EvaluateCommand inherits StackCommand {

	top: String;
	top1: String;
	top2: String;
	atoi: A2I <- new A2I;
	execute(stack : Stack, input: String) : Object {
		if (not (stack.length() = 0)) then {
			top <- stack.getTop();
			if (top = "+") then {
				top <- stack.pop();
				top1 <- stack.pop();
				top2 <- stack.pop();
				stack.push(atoi.i2a(atoi.a2i(top1) + atoi.a2i(top2)));
			} else if (top = "s") then {
				top <- stack.pop();
				top1 <- stack.pop();
				top2 <- stack.pop();
				stack.push(top1);
				stack.push(top2);
			} else 
				1
				fi
			fi;
		} 
		else 
			1
		fi
	};
};

class DisplayCommand inherits StackCommand {

	execute(stack : Stack, input: String) : Object {
		stack.display()
	};
};



