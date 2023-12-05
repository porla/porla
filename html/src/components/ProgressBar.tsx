import React from 'react';

interface ProgressBarProps {
    progress: number;
    gradient?: boolean;
}

const ProgressBar: React.FC<ProgressBarProps> = ({ progress, gradient = false }) => {
    const progressStyle = progress === 100 ? 'bg-green-600' : gradient ? 'bg-gradient-to-r from-blue-600 to-green-600' : 'bg-green-600';


    return (
        <div className="h-2 bg-neutral-800 rounded-full">
            <div className={`h-full rounded-full ${progressStyle}`} style={{ width: `${progress}%` }}></div>
        </div>
    );
};

export default ProgressBar;
