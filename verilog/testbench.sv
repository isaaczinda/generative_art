module strip_controller_testbench();

	typedef enum logic {LOADING, TESTING} statetype;
	statetype state;
	
								  
	// dut inputs
  logic clk, reset, write_en, flush;
  logic [7:0] write_data, write_addr;
  
  // dut outputs
  logic flushing, data_out;
  logic [1:0] expected;  
  
  logic [31:0]  vector_index, input_index, errors, vectors_length, inputs_length;
  
  
	
  logic [7:0]  inputs[1000:0];
  logic [1:0]  testvectors[1000:0];

  // 1/3 of an LED long
  strip_controller #(1) dut(clk, reset, write_en, flush, write_data, write_addr, flushing, data_out); 
  
  // generate clock
  always 
    begin
      clk = 1; #5; clk = 0; #5;
    end

  // at start of test, load vectors
  // and pulse reset
  initial
    begin
		state = LOADING;
		vectors_length = 466;
		inputs_length = 1;
		
		$readmemb("strip_controller.tv", testvectors);
      vector_index = 0;
		
		$readmemb("strip_controller_inputs.tv", inputs);
		input_index = 0;
		
		errors = 0;
		
		reset = 1; #18; reset = 0;
    end

  // apply test vectors on rising edge of clk
  always @(posedge clk)
	if (~reset) begin
		#1; if (state == LOADING) begin
				write_en = 1'b1;
				flush = 1'b0;
				write_data = inputs[input_index];
				$display("write data: %b", write_data);
				write_addr = input_index;
			end
		else if (state == TESTING) begin
				flush = 1'b1;
				expected = testvectors[vector_index];
				write_en = 1'b0;
				write_data = 8'hxx;
				write_addr = 8'hxx;
			end
		end


  // check results on falling edge of clk
  always @(negedge clk) 
	if (~reset) begin
		// do some checks!
		if (state == LOADING) begin
				input_index = input_index + 1;
				
				// transition if we just finished
				if (input_index == inputs_length) state = TESTING;
			end

		else if (state == TESTING) begin
				vector_index = vector_index + 1;
				
				if ({flushing, data_out} != expected) begin  // check result
						$display("Error on vector %d: outputs = %h (%h expected)", vector_index, {flushing, data_out}, expected);
						errors = errors + 1;
					end
			end


		// TODO
		if (vector_index === vectors_length) begin 
			$display("%d tests completed with %d errors", vector_index, errors);
			$stop;
		end
	end
endmodule
