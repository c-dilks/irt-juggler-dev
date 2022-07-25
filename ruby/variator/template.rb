# Template variator code
# - use this to start your own variation study
# - see other `var*.rb` files for more examples

require './ruby/variator/functions.rb'

class Variator < VariationFunctions
  def initialize

    ### PARAMETER VARIATIONS *************************
    # create the following Hash for each variation, and
    # add it to the Array `varied_settings`:
    #   {
    #     :xpath     => XPATH to the XML node
    #     :attribute => node attribute
    #     :function  => variation function (see above)
    #     :args      => variation arguments
    #     :count     => number of variations
    #   }
    @varied_settings = [
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_x',
        :function  => @@center_delta,
        :args      => [70, 20],
        :count     => 3,
      },
      {
        :xpath     => '//mirror',
        :attribute => 'focus_tune_z',
        :function  => @@min_max,
        :args      => [30, 40],
        :count     => 2,
      },
    ]


    ### FIXED SETTINGS *******************************
    # specify specific fixed settings, with similar Hashes, either of:
    #   { :constant, :value }           # for `XPATH=//constant` nodes
    #   { :xpath, :attribute, :value }  # for general attribute
    @fixed_settings = [
      { :constant=>'DRICH_debug_optics', :value=>'0' },
    ]


    ### SIMULATION COMMANDS **************************
    # - list of commands to run the simulation
    # - use `compact_file` and `output_file`, which are unique to the variant
    # - the full `simulation_pipelines` array is a list of pipelines, which will be
    #   executed sequentially
    #   - a pipeline is a list of commands, where stdout of one command is streamed
    #     to stdin of the next command
    #     - each command is written as an array, where the first element is the
    #       command, and the remaining elements are its arguments
    # - the list of pipelines will be executed for each variant
    # - example pipelines:
    #   [[ "ls", "-t" ]]                  # => `ls -t`
    #   [ ["ls","-lt"], ["tail","-n3"] ]  # => `ls -lt | tail -n3`
    @simulation_pipelines = Proc.new do |compact_file,output_file|
      [
        [[
          "./simulate.py",
          "-t 1",
          "-n 100",
          "-c #{compact_file}",
          "-o #{output_file}",
        ]],
        [[
          "./drawHits.exe",
          output_file,
        ]],
      ]
    end


    ### **********************************************

  end
  attr_accessor :varied_settings, :fixed_settings, :simulation_pipelines
end
