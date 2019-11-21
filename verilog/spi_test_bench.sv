module testbench();
	logic clk, reset, sclk, mosi, cs, flushing;
	logic [23:0] pixels, expected;
	logic [31:0]  vector_index, input_index, errors, vectors_length, inputs_length;
	
	// device under test
	neopixel_driver #(3) dut(clk, reset, sclk, mosi, cs, pixels, flushing);
	logic [1:0] inputs[1000:0];
	logic [23:0]  testvectors[1000:0];
	logic [2:0] i;
		 
    // test case
    initial begin   

    end
    
    // generate clock and load signals
    initial 
        forever begin
            clk = 1'b0; #5;
            clk = 1'b1; #5;
        end
        
    initial begin
		vectors_length = 24;
		inputs_length = 53;
		
		$readmemb("spi_to_pixels_vectors.tv", testvectors);
      vector_index = 0;
		
		$readmemb("spi_to_pixels_inputs.tv", inputs);
		input_index = 0;
		
		errors = 0;
		i = 0;
		
		reset = 1; #18; reset = 0;
    end 
    
    // shift in test vectors, wait until done, and shift out result
	always @(posedge clk)
		if (~reset) begin
			if (input_index < inputs_length) begin
			  #1; assign {cs, mosi} = inputs[input_index];
			  #1; sclk = 1; #5; sclk = 0;
			  input_index = input_index + 1;
			end else begin
				expected = testvectors[vector_index];
				if (vector_index == vectors_length) begin
					$display("Testbench ran...");
					$stop();
				end
			end
		end

	
	always @(negedge clk)
		if (~reset) begin
			vector_index = 0; // TODO
		end
endmodule