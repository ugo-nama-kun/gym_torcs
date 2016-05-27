package scr;

public abstract class Controller {

	public enum Stage {
		
		WARMUP,QUALIFYING,RACE,UNKNOWN;
		
		static Stage fromInt(int value)
		{
			switch (value) {
			case 0:
				return WARMUP;
			case 1:
				return QUALIFYING;
			case 2:
				return RACE;
			default:
				return UNKNOWN;
			}			
		}
	};
	
	private Stage stage;
	private String trackName;
	
	public float[] initAngles()	{
		float[] angles = new float[19];
		for (int i = 0; i < 19; ++i)
			angles[i]=-90+i*10;
		return angles;
	}
	
	public Stage getStage() {
		return stage;
	}
		
	public void setStage(Stage stage) {
		this.stage = stage;
	}
	
	public String getTrackName() {
		return trackName;
	}

	public void setTrackName(String trackName) {
		this.trackName = trackName;
	}

    public abstract Action control(SensorModel sensors);

    public abstract void reset(); // called at the beginning of each new trial
    
    public abstract void shutdown();

}