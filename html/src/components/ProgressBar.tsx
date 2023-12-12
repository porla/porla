interface ProgressBarProps {
  progress: number;
  gradient?: boolean
}

export const ProgressBar = ({ progress, gradient = false }: ProgressBarProps) => {
  const relativeFillPercentage = (100 / progress) * 100;

  return (
    <div className="flex items-center">
      <div className="flex-1 h-2 bg-neutral-800 rounded-xl overflow-hidden">
        <div className="h-2 rounded-xl overflow-hidden" style={{ width: `${progress}%` }}>
          <div
            className={`h-2 rounded-xl overflow-hidden ${progress === 100 ? 'bg-green-600' : gradient ? 'bg-gradient-to-r from-blue-600 to-green-600' : 'bg-blue-600'}`}
            style={{ width: `${relativeFillPercentage}%` }}
          />
        </div>
      </div>
      <div className="w-12 text-right">{`${progress}%`}</div>
    </div>
  );
};
