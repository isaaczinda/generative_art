module neopixel_driver(input logic clk, reset, sclk, mosi, cs,
					 output logic [23:0] pixels,
					 output logic flushing);
					 
	logic flush, write_en;
	logic [7:0] write_data;
	logic [7:0] write_addr;
	
	
	spi spi(sclk, mosi, cs, flush, write_en, write_data, write_addr);
	
	strip_controller #(25) strip_controller1(clk, reset, write_en, flush, write_data, write_addr, flushing, pixels[0]);

	
endmodule

module RAM(input logic clk, 
           input logic [7:0] write_addr, write_data,
			  input logic write_en,
			  input logic [7:0] read_addr,
			  output logic [7:0] read_data);

	logic [7:0] mem[255:0];
		
	always_ff @(posedge clk) begin
			read_data <= mem[read_addr];
			if (write_en) mem[write_addr] <= write_data;
		end 
endmodule


module bit_transmitter (input logic clk, reset, start, in,
								output logic out, done);

	typedef enum logic [1:0] {WAITING, WRITING} statetype;
	statetype state, next_state;
	
	// longest it needs to be is 1.25 uS == 50 cycles
	// low_level is the number of cycles we hold low
	logic [5:0] counter, next_counter, low_level;
	
	// 18 cycles == .45 uS
	// 34 cycles == .85 uS
	assign low_level = in ? 6'd18 : 6'd34;
	
	// write 0 when there's no valid output
	assign out = (state == WRITING) ? (counter > low_level) : 1'b0;
	assign done = state == WAITING;
	
	always_ff @(posedge clk, posedge reset)
		if (reset) begin
				state <= WAITING;
				
			end
		else
			begin
				state <= next_state;
				counter <= next_counter;
			end
	
	always_comb
		case (state)
			WAITING: 
				if (start) begin
					next_state = WRITING;
					next_counter = 6'd50;
				end else begin
					next_state = WAITING;
					next_counter = 6'dx; // doesn't matter
				end
			WRITING: 
				if (counter == 0) begin
					next_state = WAITING;
					next_counter = 6'dx;
				end else begin
					next_state = WRITING;
					next_counter = counter - 6'd1;
				end
		endcase
endmodule


// when start goes high, we transmit
// 
module byte_transmitter (input logic clk, reset, start,
							    input logic [7:0] in,
								 output logic out, done);
	
	typedef enum logic [1:0] {WAITING, WRITING_START, WRITING_BLOCK} statetype;
	statetype state, next_state;
	
	logic [2:0] counter, next_counter;
	
	logic bit_start, bit_in, bit_done;
	
	bit_transmitter bit_trasmitter1(clk, reset, bit_start, bit_in, out, bit_done);
	
	logic store_byte;
	logic [7:0] byte_read;
	
	
	always_ff @(posedge clk)
		if (store_byte) byte_read <= in;
		
	
	always_ff @(posedge clk, posedge reset)
		if (reset) begin
				state <= WAITING;
			end
		else
			begin 
				state <= next_state;
				counter <= next_counter;
			end
			
	assign done = state == WAITING;
	
	always_comb
		case(state)
			WAITING:
				begin
					bit_in = 1'bx;
					bit_start= 1'b0;
					
					if(start) begin
							store_byte = 1'b1;
							next_counter = 3'b000;
							next_state = WRITING_START;
							
						end
					else begin
							store_byte = 1'b0;
							next_counter = 3'bxxx;
							next_state = WAITING;
						end
				end
			
			WRITING_START:
				begin
					store_byte = 1'b0;
						
					// if we've written all of the bits out
					if (counter == 3'b111) begin
							next_state = WAITING;
							next_counter = 3'bxxx;
							bit_in = 1'bx;
							bit_start = 1'b0;
						end
					else begin
							next_state = WRITING_BLOCK;
							next_counter = counter;
							bit_in = byte_read[counter];
							bit_start = 1'b1;
						end
				end
			
			WRITING_BLOCK:
				// if the bit controller finished writing its bit
				if (bit_done) begin
						store_byte = 1'b0;
						next_state = WRITING_START;
						next_counter = counter + 3'b001;
						bit_in = 1'bx; // no need to set this valid here
						bit_start = 1'b1;
					end
				// if the bit controller is still writing its bit
				else begin
						store_byte = 1'b0;
						next_state = WRITING_BLOCK;
						next_counter = counter;
						bit_in = byte_read[counter];
						bit_start = 1'b0;
					end
		endcase
endmodule
								 
			  
module strip_controller #(parameter STRIP_LENGTH)
                         (input logic clk, reset, write_en, flush, 
                          input logic [7:0] write_data, write_addr,
								  output logic flushing, data_out);
	
	logic [7:0] read_addr, next_read_addr, read_data;
	
	RAM strip_data(clk, write_addr, write_data, write_en, next_read_addr, read_data);
	
	logic byte_start, byte_out, byte_done, next_byte_start;
	// Counts to 2000 cycles = 50 microseconds
	logic [10:0] waiting_counter, next_waiting_counter;
	
	
	byte_transmitter byte_transmitter1(clk, reset, byte_start, read_data, byte_out, byte_done);
	
	// when flush goes high, enter "flushing" state
	typedef enum logic [1:0] {LOADING, FLUSHING_START, FLUSHING_BLOCK, WAITING} statetype;
	statetype state, next_state;
	
	always_ff @(posedge clk, posedge reset)
		if (reset) begin
				state <= LOADING;
				read_addr <= 8'hxx;
				byte_start <= 1'bx;
				waiting_counter <= 11'bx;
			end
		else
			begin
				state <= next_state;
				read_addr <= next_read_addr;
				byte_start <= next_byte_start;
				waiting_counter <= next_waiting_counter;
			end
			
	assign flushing = (state == FLUSHING_START || state == FLUSHING_BLOCK || state == WAITING);
	
//	// Should always output 0 when not flushing (hopefully redundant)
//	assign data_out = (state == FLUSHING_START) ? byte_out : 1'b0;
	assign data_out = byte_out;
	
	always_comb
		case(state)
			LOADING:
				if(flush) begin
						next_state = FLUSHING_START;
						next_read_addr = 8'h00;
						next_byte_start = 1'b1;
						next_waiting_counter = 11'bx;
					end
				else begin
						next_state = LOADING;
						next_read_addr = 8'hxx;
						next_byte_start = 1'b0;
						next_waiting_counter = 11'bx;
					end
			FLUSHING_START:
				begin
					if (read_addr == (STRIP_LENGTH)) begin
							next_state = WAITING;
							next_read_addr = 8'hxx;
							next_byte_start = 1'b0;
							next_waiting_counter = 11'b0;
						end
					else begin
							next_state = FLUSHING_BLOCK;
							next_read_addr = read_addr;
							next_byte_start = 1'b0;
							next_waiting_counter = 11'bx;
						end
				end
			FLUSHING_BLOCK:
				if(byte_done) begin
						next_state = FLUSHING_START;
						next_read_addr = read_addr + 8'h01;;
						next_byte_start = 1'b1;
						next_waiting_counter = 11'bx;
					end
				else begin
						next_state = FLUSHING_BLOCK;
						next_read_addr = read_addr;
						next_byte_start = 1'b0;
						next_waiting_counter = 11'bx;
					end
			WAITING:
				if (waiting_counter == 11'd2000) begin
						next_state = LOADING;
						next_read_addr = 8'hxx;
						next_byte_start = 1'b0;
						next_waiting_counter = 11'bx;
					end
				else begin
						next_state = WAITING;
						next_read_addr = 8'hxx;
						next_byte_start = 1'b0;
						next_waiting_counter = waiting_counter + 11'b1;
					end
			default: begin
					next_state = state;
					next_read_addr = 8'hxx;
					next_byte_start = 1'bx;
					next_waiting_counter = 11'bx;
				end
		endcase
endmodule
						  
			  

module spi(input logic sclk, mosi, cs,
			  output logic flush, write_en, 
			  output logic [7:0] write_data,
			  output logic [7:0] write_offset,
			  output logic [4:0] strip_num);

	logic [7:0] byte_read;
	logic strip_num_en, offset_en, offset_incr;
	
				  
	always_ff @(posedge sclk)
		byte_read <= {byte_read[6:0], mosi};
	
	
	spi_controller(sclk, cs, byte_read, strip_num_en, offset_en, write_en, offset_incr, flush);
	
	
	always_ff @(posedge sclk) 
		if (strip_num_en) strip_num <= byte_read[4:0];
	
	
	logic [7:0] offset;
	
	// offset holds the offset from the address pointer 
	// where we will write data
	always_ff @(posedge sclk) 
		if (offset_en) begin
				if (offset_incr) offset <= offset + 8'b1;
				else offset <= byte_read;
			end
	
	
	
			  
endmodule

module spi_controller(input logic sclk, cs,
							 input logic [7:0] byte_read,
							 output logic strip_num_en, offset_en, write_en, offset_incr, flush);
							 
	logic [2:0] byte_counter;
	logic byte_done;
	
	typedef enum logic [1:0] {RESET, FLUSH, READ_OFFSET, READ_DATA} statetype;
	statetype state, next_state;
	
	assign byte_done = (byte_counter == 3'd7);
	
	always_ff @(posedge sclk)
		if (cs) byte_counter <= byte_counter + 3'b1;
		else byte_counter <= 3'b000;

	always_ff @(posedge sclk)
		if (cs && byte_done) state <= next_state;
		else state <= RESET;	
	
	
	always_comb
		if (byte_done)
			case(state)
				RESET: if(byte_read == 8'b01000110) next_state = FLUSH;
						 else if(byte_read < 8'd24) next_state = READ_OFFSET;
						 else next_state = RESET;
				READ_OFFSET: next_state = READ_DATA;
				READ_DATA: next_state = READ_DATA;
				default: next_state = RESET;
			endcase
		
		else
			next_state = state;
	
	always_comb begin
		if (byte_done)
			case (next_state)
			
				RESET: begin
						strip_num_en = 0;
						offset_en = 0;
						write_en = 0;
						offset_incr = 0;
						flush = 0;
					end
					
				FLUSH: begin
						strip_num_en = 0;
						offset_en = 0;
						write_en = 0;
						offset_incr = 0;
						flush = 1;
					end
				
				READ_OFFSET: begin
						strip_num_en = 1;
						offset_en = 0;
						write_en = 0;
						offset_incr = 0;
						flush = 0;
					end
				
				READ_DATA: begin
						// if we are moving from READ_OFFSET --> READ_DATA
						// we want to save the offset in a register
						if (state == READ_OFFSET) begin 
								strip_num_en = 0;
								offset_en = 1;
								write_en = 0;
								offset_incr = 0;
								flush = 0;
							end 
						else begin
								strip_num_en = 0;
								offset_en = 1;
								write_en = 1;
								offset_incr = 1;
								flush = 0;
							end				
					end
				
				default: begin
						strip_num_en = 1'bx;
						offset_en = 1'bx;
						write_en = 1'bx;
						offset_incr = 1'bx;
						flush = 1'bx;
					end
				endcase
			
		else begin
				strip_num_en = 0;
				offset_en = 0;
				write_en = 0;
				offset_incr = 0;
				flush = 0;
			end
	end				 
endmodule
