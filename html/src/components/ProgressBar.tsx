type GradientProps = {
  progress: number;
  gradient: true;
  fromColor: string;
  toColor: string;
};

type NonGradientProps = {
  progress: number;
  gradient?: false;
  fromColor?: never;
  toColor?: never;
};

type ProgressBarProps = GradientProps | NonGradientProps;

export const ProgressBar = ({ progress, ...props }: ProgressBarProps) => {
  const relativeFillPercentage = (100 / progress) * 100;
  const { gradient = false, fromColor, toColor } = props;

  return (
    <div className="flex items-center">
      <div className="flex-1 h-2 bg-neutral-800 rounded-xl overflow-hidden">
        <div className="h-2 rounded-xl overflow-hidden" style={{ width: `${progress}%` }}>
          <div
            className={`h-2 rounded-xl overflow-hidden ${progress === 100 ? 'bg-green-600' : gradient ? `bg-gradient-to-r ${fromColor} ${toColor}` : 'bg-blue-600'}`}
            style={{ width: `${relativeFillPercentage}%` }}
          />
        </div>
      </div>
      <div className="w-12 text-right">{`${progress}%`}</div>
    </div>
  );
};
