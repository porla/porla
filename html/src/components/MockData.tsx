import { CloudArrowDownIcon, CloudArrowUpIcon, CloudIcon, ExclamationTriangleIcon } from "@heroicons/react/24/outline";
import PorlaLogo from '../porla.svg?react'

export const navigation = [

  { name: 'All', href: '#', icon: CloudIcon, current: true, count: 122 },
  { name: 'Downloading', href: '#', icon: CloudArrowDownIcon, current: false, count: 33 },
  { name: 'Seeding', href: '#', icon: CloudArrowUpIcon, current: false, count: 89 },
  { name: 'Error', href: '#', icon: ExclamationTriangleIcon, current: false, count: 0 },

]

export const trackers = [

  { url: "https://tracker1.porla.org/announce/s0m3p455k3y", href: '#', icon: PorlaLogo, current: true, count: 2 },
  { url: "https://tracker2.porla.org/announce/4n0th3rp455k3y", href: '#', icon: PorlaLogo, current: false, count: 0 }

]

export const torrents = [

  {
    id: 1,
    name: 'debian-12.2.0-amd64-netinst.iso',
    size: '287.4 MiB',
    progress: 99,
    status: 'Downloading',
    download_speed: '117.00 MiB/s',
    upload_speed: '117.00 MiB/s',
    active_seeds: 1,
    inactive_seeds: 2,
    active_peers: 3,
    inactive_peers: 4,
    ratio: 13.37
  },
  {
    id: 2,
    name: "archlinux-2023.12.01-x86_64.iso",
    size: "872 MiB",
    progress: 100,
    status: 'Seeding',
    download_speed: '1.17 GiB/s',
    upload_speed: '1.17 GiB/s',
    active_seeds: 5,
    inactive_seeds: 6,
    active_peers: 7,
    inactive_peers: 8,
    ratio: 133.7
  },
  {
    id: 3,
    name: "ubuntu-20.04.6-live-server-amd64.iso",
    size: "1.4 GiB",
    progress: 50,
    status: 'Downloading',
    download_speed: '11.70 MiB/s',
    upload_speed: '11.70 MiB/s',
    active_seeds: 9,
    inactive_seeds: 10,
    active_peers: 11,
    inactive_peers: 12,
    ratio: 1.337
  }

]
