export default function prefixPath(path: string) {
  let base = (window as any).porla.base_path as string;
  if (base.endsWith("/")) base = base.substring(0, base.length - 1);
  return `${base}${path}`;
}
